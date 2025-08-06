#include "shell.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_heredoc
{
	char *content;
	size_t len;
} t_heredoc;

typedef struct s_heredoc_buffer
{
	char *content;
	char *line;
	char *line_with_nl;
	char *new_content;
	size_t delimiter_len;
} t_heredoc_buffer;

typedef struct s_reader
{
	char *buf;
	size_t size;
	size_t pos;
} t_reader;

char *ft_strcpy(char *dest, const char *src)
{
	size_t i;

	i = 0;
	while (src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return dest;
}

char *ft_strcat(char *dest, const char *src)
{
	size_t i;
	size_t j;

	j = 0;
	i = 0;
	while (dest[i])
		i++;
	while (src[j])
	{
		dest[i + j] = src[j];
		j++;
	}
	dest[i + j] = '\0';
	return (dest);
}

typedef struct s_exec_data
{
	int in_fd;
	int pipefd[2];
	int *pids;
	int i;
	int pids_size;
	char **env;
	int last_exit_status;
	int original_stdin;
	int original_stdout;
	int return_status;
} t_exec_data;

char *copy_dir(const char *dir, int len)
{
	char *res;
	int i;

	i = 0;
	res = gb_malloc(len + 1);
	if (!res)
		return (NULL);
	while (i < len)
	{
		res[i] = dir[i];
		i++;
	}
	res[len] = '\0';
	return (res);
}

char *join_path(char *dir, char *file)
{
	size_t dlen;
	size_t flen;
	char *res;

	flen = strlen(file);
	dlen = strlen(dir);
	res = gb_malloc(dlen + flen + 2);
	if (!res)
		return (NULL);
	ft_memcpy(res, dir, dlen);
	res[dlen] = '/';
	ft_memcpy(res + dlen + 1, file, flen);
	res[dlen + flen + 1] = '\0';
	return (res);
}

int is_exec_other(char *path)
{
	struct stat path_stat;

	if (access(path, F_OK) != 0)
		return (0);

	if (stat(path, &path_stat) != 0)
		return (0);

	if (S_ISDIR(path_stat.st_mode))
		return (0);

	if (access(path, X_OK) != 0)
		return (0);
	return (1);
}

char *check_cmd_in_dir(const char *dir_start, int len, char *cmd)
{
	char *result;
	char *dir;

	dir = copy_dir(dir_start, len);
	if (!dir)
		return (NULL);
	result = join_path(dir, cmd);
	//free(dir);
	if (is_exec_other(result))
		return (result);
	//free(result);
	return (NULL);
}

char *search_in_path(char *cmd, char *path)
{
	char *start;
	char *finish;
	char *found;
	int len;

	start = path;
	finish = path;
	while (1)
	{
		if (*start == ':' || *start == '\0')
		{
			len = start - finish;
			if (len > 0)
			{
				found = check_cmd_in_dir(finish, len, cmd);
				if (found) return found;
			}
			if (*start == '\0')
				break;
			finish = start + 1;
		}
		start++;
	}
	return (NULL);
}

char *find_executable(char *cmd)
{
	char *path;

	if (is_exec_other(cmd))
		return (ft_strdup(cmd));
	path = getenv("PATH");
	if (!path)
		return (NULL);
	return (search_in_path(cmd, path));
}

int redir_in(t_redirection *redir)
{
	int fd;

	fd = open(redir->filename, O_RDONLY);
	if (fd < 0)
	{
		perror(redir->filename);
		return (-1);
	}
	dup2(fd, STDIN_FILENO);
	close(fd);
	return (0);
}

int redir_out(t_redirection *redir)
{
	int fd;

	fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		perror(redir->filename);
		return (-1);
	}
	dup2(fd, STDOUT_FILENO);
	close(fd);
	return (0);
}

int redir_append(t_redirection *redir)
{
	int fd;

	fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, 0777);
	if (fd < 0)
	{
		perror(redir->filename);
		return (-1);
	}
	dup2(fd, STDOUT_FILENO);
	close(fd);
	return (0);
}

char *expand_buf(t_reader *r, size_t new_size)
{
	char *new_buf;
	size_t i;

	new_buf = gb_malloc(new_size);
	if (!new_buf)
		return NULL;
	i = 0;
	while (i < r->pos)
	{
		new_buf[i] = r->buf[i];
		i++;
	}
	gc_free(r->buf);
	r->buf = new_buf;
	r->size = new_size;
	return (r->buf);
}

int add_char(t_reader *r, char c)
{
	size_t new_size;

	if (r->pos + 1 >= r->size)
	{
		new_size = r->size * 2;
		if (new_size < r->size + 1)
			new_size = r->size + 1;
		if (!expand_buf(r, new_size))
			return (0);
	}
	r->buf[r->pos++] = c;
	return (1);
}

char *ft_readline_loop(t_reader *r)
{
	char c;
	ssize_t n;

	while ((n = read(STDIN_FILENO, &c, 1)) > 0)
	{
		if (c == '\n')
			break;
		if (!add_char(r, c))
		{
			gc_free(r->buf);
			return NULL;
		}
	}
	if (n <= 0 && r->pos == 0)
	{
		gc_free(r->buf);
		return NULL;
	}
	if (r->pos + 1 >= r->size && !expand_buf(r, r->pos + 1))
		return NULL;
	r->buf[r->pos] = '\0';
	return (r->buf);
}

char *ft_readline(char *prompt)
{
	t_reader r;
	r.size = 1;
	r.pos = 0;
	r.buf = gb_malloc(r.size);
	if (!r.buf)
		return NULL;
	if (prompt)
		write(STDOUT_FILENO, prompt, ft_strlen(prompt));
	return (ft_readline_loop(&r));
}

int heredoc_append_line(t_heredoc_buffer *buf)
{
	size_t line_len;

	line_len = ft_strlen(buf->line);
	buf->line_with_nl = gb_malloc(line_len + 2);
	if (!buf->line_with_nl)
		return 0;
	ft_strcpy(buf->line_with_nl, buf->line);
	buf->line_with_nl[line_len] = '\n';
	buf->line_with_nl[line_len + 1] = '\0';
	buf->new_content = gb_malloc(ft_strlen(buf->content) + ft_strlen(buf->line_with_nl) + 1);
	if (!buf->new_content)
	{
		gc_free(buf->line_with_nl);
		return 0;
	}
	ft_strcpy(buf->new_content, buf->content);
	ft_strcat(buf->new_content, buf->line_with_nl);
	gc_free(buf->content);
	gc_free(buf->line_with_nl);
	buf->content = buf->new_content;
	return (1);
}

char *expand_heredoc_line_impl(char *line, t_env *env_list, int exit_status)
{
	char *expanded;

	if (!line)
		return NULL;
	expanded = expand_with_quotes(line, env_list, exit_status);
	if (expanded)
		return expanded;
	else
		return ft_strdup(line);
}

int heredoc_append_expanded(t_heredoc_buffer *buf, t_env *env_list, int exit_status)
{
	char *expanded_line;
	char *old_line;
	int result;

	old_line = buf->line;
	expanded_line = expand_heredoc_line_impl(buf->line, env_list, exit_status);
	buf->line = expanded_line;
	result = heredoc_append_line(buf);
	buf->line = old_line;
	if (expanded_line != old_line)
		gc_free(expanded_line);
	return result;
}

char *readline_loop(t_heredoc_buffer *buf, const char *delimiter)
{
	heredoc_signals();

	while (1)
	{
		buf->line = ft_readline("> ");
		if (!buf->line)
		{
			setup_interactive_signals();
			break;
		}
		if ((size_t)ft_strlen(buf->line) == buf->delimiter_len &&
			ft_strcmp(buf->line, delimiter) == 0)
		{
			gc_free(buf->line);
			break;
		}
		if (!heredoc_append_line(buf))
		{
			perror("Satır eklenemedi");
			gc_free(buf->line);
			gc_free(buf->content);
			setup_interactive_signals();
			return NULL;
		}
		gc_free(buf->line);
		buf->line = NULL;
	}
	setup_interactive_signals();
	return buf->content;
}

char *readline_loop_expand(t_heredoc_buffer *buf, const char *delimiter,
						  t_env *env_list, int exit_status)
{
	heredoc_signals();

	while (1)
	{
		buf->line = ft_readline("> ");
		if (!buf->line)
		{
			setup_interactive_signals();
			break;
		}
		if ((size_t)ft_strlen(buf->line) == buf->delimiter_len &&
			ft_strcmp(buf->line, delimiter) == 0)
		{
			gc_free(buf->line);
			break;
		}
		if (!heredoc_append_expanded(buf, env_list, exit_status))
		{
			perror("Satır eklenemedi");
			gc_free(buf->line);
			gc_free(buf->content);
			setup_interactive_signals();
			return NULL;
		}
		gc_free(buf->line);
		buf->line = NULL;
	}
	setup_interactive_signals();
	return buf->content;
}
char *read_single_heredoc_block(char *delimiter)
{
	t_heredoc_buffer buf;
	if (!delimiter || *delimiter == '\0')
	{
		printf("Hata: Heredoc ayirici kelimesi belirtilmedi.\n");
		return NULL;
	}
	buf.content = ft_strdup("");
	buf.line = NULL;
	buf.line_with_nl = NULL;
	buf.new_content = NULL;
	buf.delimiter_len = ft_strlen(delimiter);
	if (!buf.content)
	{
		perror("Heredoc bellek ayirma basarisiz");
		return NULL;
	}
	char *result = readline_loop(&buf,delimiter);
	if (result)
		return result;
	else
		return buf.content;
}

char *read_heredoc_with_expand(char *delimiter, t_env *env_list, int exit_status)
{
	t_heredoc_buffer buf;
	if (!delimiter || *delimiter == '\0')
	{
		printf("Hata: Heredoc ayirici kelimesi belirtilmedi.\n");
		return NULL;
	}
	buf.content = ft_strdup("");
	buf.line = NULL;
	buf.line_with_nl = NULL;
	buf.new_content = NULL;
	buf.delimiter_len = ft_strlen(delimiter);
	if (!buf.content)
	{
		perror("Heredoc bellek ayirma basarisiz");
		return NULL;
	}
	char *result = readline_loop_expand(&buf, delimiter, env_list, exit_status);
	if (result)
		return result;
	else
		return buf.content;
}

int ft_h_built_expand(t_redirection *current_redir, char *heredoc_content,
					 int *last_heredoc_fd, size_t *heredoc_len,
					 t_env *env_list, int exit_status)
{
	int pipefd[2];
	if (current_redir->no_expand)
		heredoc_content = read_single_heredoc_block(current_redir->filename);
	else
		heredoc_content = read_heredoc_with_expand(current_redir->filename, env_list, exit_status);

	if (!heredoc_content)
	{
		if (*last_heredoc_fd != -2 && *last_heredoc_fd != -1)
			close(*last_heredoc_fd);
		return -1;
	}
	*heredoc_len = ft_strlen(heredoc_content);
	if (pipe(pipefd) == -1)
	{
		perror("heredoc pipe");
		gc_free(heredoc_content);
		return -1;
	}
	write(pipefd[1], heredoc_content, *heredoc_len);
	close(pipefd[1]);
	gc_free(heredoc_content);
	*last_heredoc_fd = pipefd[0];
	return pipefd[0];
}

int h_loop(t_parser *cmd,char *heredoc_content,int *last_heredoc_fd,size_t *heredoc_len, t_env *env_list, int exit_status)
{
	t_redirection *current_redir;

	current_redir = cmd->redirs;
	while (current_redir)
	{
		if (current_redir->type == REDIR_HEREDOC)
		{
			if (heredoc_content)
			{
				gc_free(heredoc_content);
				heredoc_content = NULL;
			}
			if (*last_heredoc_fd != -2 && *last_heredoc_fd != -1)
			{
				close(*last_heredoc_fd);
				*last_heredoc_fd = -1;
			}
			if(ft_h_built_expand(current_redir,heredoc_content,last_heredoc_fd,heredoc_len, env_list, exit_status) == -1)
				return (-1);
		}
		current_redir = current_redir->next;
	}
return (0);
}

int process_heredocs(t_parser *cmd, t_env *env_list, int exit_status)
{
	int last_heredoc_fd;
	size_t heredoc_len;
	char *heredoc_content;

	heredoc_content = NULL;
	heredoc_len = 0;
	last_heredoc_fd = -2;
	if(h_loop(cmd,heredoc_content,&last_heredoc_fd,&heredoc_len, env_list, exit_status) == -1)
		return (-1);
	return (last_heredoc_fd);
}

int setup_file_redirections(t_parser *cmd)
{
	t_redirection *current_redir;

	current_redir = cmd->redirs;
	while (current_redir)
	{
		if (current_redir->type == REDIR_IN)
		{
			if (redir_in(current_redir) != 0)
				return (-1);
		}
		else if (current_redir->type == REDIR_OUT)
		{
			if (redir_out(current_redir) != 0)
				return (-1);
		}
		else if (current_redir->type == REDIR_APPEND)
		{
			if (redir_append(current_redir) != 0)
				return (-1);
		}
		current_redir = current_redir->next;
	}
	return (0);
}

int ft_builtin_call(t_parser *cmd, t_exec_data *data, t_env **env_list)
{
	if (ft_strcmp(cmd->argv[0], "echo") == 0)
	{
		built_echo(cmd);
		data->last_exit_status = 0;
		return (0);
	}
	if (ft_strcmp(cmd->argv[0], "cd") == 0)
	{
		int cd_result = built_cd(cmd);
		data->last_exit_status = cd_result;
		return (0);
	}
	if (ft_strcmp(cmd->argv[0], "pwd") == 0)
	{
		builtin_pwd();
		data->last_exit_status = 0;
		return (0);
	}
	if (ft_strcmp(cmd->argv[0], "exit") == 0)
	{
		builtin_exit(cmd);
		return (0);
	}
	if (ft_strcmp(cmd->argv[0], "export") == 0)
	{
		builtin_export(cmd, env_list);
		data->last_exit_status = 0;
		return (0);
	}
	if (ft_strcmp(cmd->argv[0], "unset") == 0)
	{
		builtin_unset(cmd, env_list);
		data->last_exit_status = 0;
		return (0);
	}
	if (ft_strcmp(cmd->argv[0], "env") == 0)
	{
		builtin_env(*env_list);
		data->last_exit_status = 0;
		return (0);
	}
	return 1;
}

int is_builtin(t_parser *cmd)
{
	if (!cmd || !cmd->argv || !cmd->argv[0])
		return (0);
	if (strcmp(cmd->argv[0], "exit") == 0 || strcmp(cmd->argv[0], "cd") == 0)
		return (1);
	if (strcmp(cmd->argv[0], "pwd") == 0 || strcmp(cmd->argv[0], "echo") == 0)
		return (1);
	if (strcmp(cmd->argv[0], "export") == 0 || strcmp(cmd->argv[0], "unset") == 0)
		return (1);
	if (strcmp(cmd->argv[0], "env") == 0)
		return (1);
	return 0;
}

int count_commands(t_parser *cmd)
{
	int count;

	count = 0;
	while (cmd)
	{
		count++;
		cmd = cmd->next;
	}
	return (count);
}
int ft_redir_ctrl(t_parser *cmd)
{
	t_redirection *redir;

	redir = cmd->redirs;
	while (redir)
	{
		if (redir->type == REDIR_IN)
		{
			if (redir_in(redir) != 0)
				return (-1);
		}
		else if (redir->type == REDIR_OUT)
		{
			if (redir_out(redir) != 0)
				return (-1);
		}
		else if (redir->type == REDIR_APPEND)
		{
			if (redir_append(redir) != 0)
				return (-1);
		}
		else if (redir->type == REDIR_HEREDOC)
		{
			if (cmd->heredoc_fd != -1 && cmd->heredoc_fd != -2)
			{
				if (dup2(cmd->heredoc_fd, STDIN_FILENO) == -1)
				{
					perror("dup2 cmd->heredoc_fd");
					return (-1);
				}
				close(cmd->heredoc_fd);
				cmd->heredoc_fd = -1;
			}
		}
		redir = redir->next;
	}
	return (0);
}
void ft_exec_start(t_parser *cmd,t_exec_data *data, t_env **env_list)
{
	char *exec_path;
	if (is_builtin(cmd))
	{
		ft_builtin_call(cmd, data, env_list);
		exit(data->last_exit_status);
	}
	else
	{
        
		struct stat path_stat;
		if (stat(cmd->argv[0], &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
		{
        
			printf("%s: is a directory\n", cmd->argv[0]);
			exit(126);
		}

		exec_path = find_executable(cmd->argv[0]);
		if (!exec_path)
		{
            
			printf("%s: command not found\n", cmd->argv[0]);
            gb_free_all();
            env_gb_free_all();
			exit(127);
		}
     
		execve(exec_path, cmd->argv, data->env);
		perror("execve");
		exit(1);
	}
}

void child_process_exec(t_parser *cmd, t_exec_data *data, t_env **env_list)
{
	if (data->original_stdin != -1)
		close(data->original_stdin);
	if (data->original_stdout != -1)
		close(data->original_stdout);

	setup_child_signals();

	if (data->in_fd != STDIN_FILENO)
	{
		if (dup2(data->in_fd, STDIN_FILENO) == -1)
		{
			perror("dup2 in_fd");
			exit(1);
		}
		close(data->in_fd);
	}
	if (cmd->next)
	{
		if (dup2(data->pipefd[1], STDOUT_FILENO) == -1)
		{
			perror("dup2 pipefd[1]");
			exit(1);
		}
		close(data->pipefd[0]);
		close(data->pipefd[1]);
	}
	if (ft_redir_ctrl(cmd) == -1)
	{
        env_gb_free_all();
        gb_free_all();
		exit(1);
	}
	ft_exec_start(cmd, data, env_list);
}

int setup_pipe_if_needed(t_parser *cmd, t_exec_data *data)
{
	if (!cmd->next)
		return (0);
	if (pipe(data->pipefd) == -1)
	{
		perror("pipe");
		return -1;
	}
	return 0;
}

int fork_pipe_exec(t_parser *cmd, t_exec_data *data, t_env **env_list)
{
	int pid;

	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		return -1;
	}
	if (pid == 0)
	{
		child_process_exec(cmd, data, env_list);
		exit(1);
	}
	return (pid);
}

int setup_and_fork(t_parser *cmd, t_exec_data *data, t_env **env_list)
{
	int pid;

	if (setup_pipe_if_needed(cmd, data) == -1)
		return (-1);
	pid = fork_pipe_exec(cmd, data, env_list);
	if (pid == -1)
	{
		if (cmd->next)
		{
			close(data->pipefd[0]);
			close(data->pipefd[1]);
		}
		return -1;
	}

	if (data->i >= data->pids_size)
	{
		printf("limit error\n");
		return (-1);
	}
	data->pids[data->i++] = pid;
	return (pid);
}

int process_command(t_parser *cmd, t_exec_data *data, t_env **env_list)
{
	if (setup_and_fork(cmd, data, env_list) == -1)
		return 0;
	if (data->in_fd != STDIN_FILENO)
		close(data->in_fd);
	if (cmd->next)
	{
		close(data->pipefd[1]);
		data->in_fd = data->pipefd[0];
	}
	else
		data->in_fd = STDIN_FILENO;
	return 1;
}

void execute_loop(t_parser *cmd_list, t_exec_data *data, t_env **env_list)
{
	data->in_fd = STDIN_FILENO;
	t_parser *cmd;

	cmd = cmd_list;
	while (cmd)
	{
		if (!process_command(cmd, data, env_list))
			break;
		cmd = cmd->next;
	}
	if (data->in_fd != STDIN_FILENO)
		close(data->in_fd);
}

void wait_pids(t_exec_data *data)
{
	int status;
	int j;

	j = 0;
	while (j < data->i)
	{
		waitpid(data->pids[j], &status, 0);
		if (j == data->i - 1)
			data->last_exit_status = calculate_exit_status(status);
		j++;
	}
}

int heredoc_fd_error(t_parser *cmd_list,t_exec_data *data,t_parser *current_cmd)
{
	t_parser *tmp_cmd;

	dup2(data->original_stdin, STDIN_FILENO);
	close(data->original_stdin);
	dup2(data->original_stdout, STDOUT_FILENO);
	close(data->original_stdout);
	tmp_cmd = cmd_list;
	while(tmp_cmd != current_cmd->next)
	{
		if (tmp_cmd && tmp_cmd->heredoc_fd != -1 && tmp_cmd->heredoc_fd != -2)
		{
			close(tmp_cmd->heredoc_fd);
			tmp_cmd->heredoc_fd = -1;
		}
		tmp_cmd = tmp_cmd->next;
	}
	return (1);

}
int heredoc_handle(t_parser *cmd_list,t_exec_data *data, t_env *env_list, int exit_status)
{
	t_parser *current_cmd;

	current_cmd = cmd_list;
	while (current_cmd)
	{
		current_cmd->heredoc_fd = process_heredocs(current_cmd, env_list, exit_status);
		if (current_cmd->heredoc_fd == -1)
		{
			data->last_exit_status = 130;
			heredoc_fd_error(cmd_list,data,current_cmd);
			return (1);
		}
		current_cmd = current_cmd->next;
	}
	return (0);
}
int n_next_or_built(t_parser *cmd_list, t_exec_data *data, t_env **env_list)
{
	if (cmd_list->heredoc_fd != -1 && cmd_list->heredoc_fd != -2)
	{
		if (dup2(cmd_list->heredoc_fd, STDIN_FILENO) == -1)
		{
			perror("dup2 heredoc for builtin");
			data->return_status = 1;
		}
		close(cmd_list->heredoc_fd);
		cmd_list->heredoc_fd = -1;
	}
	if (data->return_status == 0)
	{
		if (setup_file_redirections(cmd_list) != 0)
		{
			data->last_exit_status = 1;
		}
		else
		{
			ft_builtin_call(cmd_list, data, env_list);
		}
	}
	dup2(data->original_stdin, STDIN_FILENO);
	close(data->original_stdin);
	dup2(data->original_stdout, STDOUT_FILENO);
	close(data->original_stdout);
	return (data->last_exit_status);

}
int ft_data_pids(t_parser *cmd_list, t_exec_data *data, t_env **env_list)
{
	t_parser *current_cmd;

	current_cmd = cmd_list;
	data->pids = gb_malloc(sizeof(int) * data->pids_size);
	if (!data->pids)
	{
		perror("pids error\n");
		current_cmd = cmd_list;
		while(current_cmd)
		{
			if (current_cmd->heredoc_fd != -1 && current_cmd->heredoc_fd != -2)
			{
				close(current_cmd->heredoc_fd);
				current_cmd->heredoc_fd = -1;
			}
			current_cmd = current_cmd->next;
		}
		dup2(data->original_stdin, STDIN_FILENO);
		close(data->original_stdin);
		dup2(data->original_stdout, STDOUT_FILENO);
		close(data->original_stdout);
		return (1);
	}
	execute_loop(cmd_list, data, env_list);
	wait_pids(data);
	gc_free(data->pids);
	return (0);
}
void finish_fd(t_parser *cmd_list,t_exec_data *data)
{
	t_parser *current_cmd;

	current_cmd = cmd_list;
	while(current_cmd)
	{
		if (current_cmd->heredoc_fd != -1 && current_cmd->heredoc_fd != -2) {
			close(current_cmd->heredoc_fd);
			current_cmd->heredoc_fd = -1;
		}
		current_cmd = current_cmd->next;
	}
	dup2(data->original_stdin, STDIN_FILENO);
	close(data->original_stdin);
	dup2(data->original_stdout, STDOUT_FILENO);
	close(data->original_stdout);

}
int execute_cmds(t_parser *cmd_list, char **env, t_env **env_list)
{
	t_exec_data data;
	int pid_len;
	data.return_status = 0;
	data.last_exit_status = 0;
	data.original_stdin = dup(STDIN_FILENO);
	data.original_stdout = dup(STDOUT_FILENO);
	cmd_list->fd_i = data.original_stdin;
	cmd_list->fd_o = data.original_stdout;

	if (data.original_stdin == -1 || data.original_stdout == -1)
	{
		perror("dup failed");
		if (data.original_stdin != -1) 
            close(data.original_stdin);
		if (data.original_stdout != -1) 
            close(data.original_stdout);
		return 1;
	}

	if (heredoc_handle(cmd_list, &data, *env_list, data.last_exit_status))
		return (data.last_exit_status);

	pid_len = count_commands(cmd_list);
	data.pids_size = pid_len;
	data.pids = NULL;
	data.i = 0;
	data.env = env;
	data.in_fd = STDIN_FILENO;

	if (pid_len == 1 && is_builtin(cmd_list))
	{
		int builtin_exit_code = n_next_or_built(cmd_list, &data, env_list);
		data.last_exit_status = builtin_exit_code;
		return (data.last_exit_status);
	}

	setup_parent_execution_signals();

	if (ft_data_pids(cmd_list, &data, env_list))
	{
		setup_interactive_signals();
		return (1);
	}

	setup_interactive_signals();

	finish_fd(cmd_list, &data);

	return (data.last_exit_status);
}