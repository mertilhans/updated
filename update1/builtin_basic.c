#include "shell.h"

// -n flag'inin geçerli olup olmadığını kontrol et
static int is_valid_n_flag(char *arg)
{
    int j;
    
    if (!arg || arg[0] != '-' || arg[1] != 'n')
        return 0;
    
    j = 2;
    while (arg[j])
    {
        if (arg[j] != 'n')
            return 0;
        j++;
    }
    return 1;
}

// echo argümanlarının başlangıç indexini bul ve newline durumunu belirle
static int process_echo_flags(t_parser *cmd, int *newline)
{
    int i;
    
    i = 1;
    *newline = 1;
    
    while (cmd->argv[i] && is_valid_n_flag(cmd->argv[i]))
    {
        *newline = 0;
        i++;
    }
    return i;
}

// echo argümanlarını yazdır
static void print_echo_args(t_parser *cmd, int start_index)
{
    int i;
    
    i = start_index;
    while (cmd->argv[i])
    {
        printf("%s", cmd->argv[i]);
        if (cmd->argv[i + 1] != NULL)
            printf(" ");
        i++;
    }
}

// echo komutu - -n flag'i de destekliyor
void built_echo(t_parser *cmd)
{
    int start_index;
    int newline;
    
    start_index = process_echo_flags(cmd, &newline);
    print_echo_args(cmd, start_index);
    
    if (newline)
        printf("\n");
}

// cd için memory temizleme helper fonksiyonu
static void cleanup_cd_memory(char *current_pwd, char *old_pwd)
{
    if (current_pwd)
        free(current_pwd);
    if (old_pwd)
        gc_free(old_pwd);
}

// cd argüman sayısını kontrol et
static int validate_cd_args(t_parser *cmd)
{
    int arg_count;
    
    arg_count = 0;
    while (cmd->argv[arg_count])
        arg_count++;
    
    if (arg_count > 2)
    {
        printf("bash: cd: too many arguments\n");
        return 1;
    }
    return 0;
}

// HOME directory'ye git
static int change_to_home(char *current_pwd, char *old_pwd)
{
    char *home_dir;
    
    home_dir = getenv("HOME");
    if (home_dir == NULL)
    {
        printf("cd: HOME not set\n");
        cleanup_cd_memory(current_pwd, old_pwd);
        return 1;
    }
    if (chdir(home_dir) != 0)
    {
        perror("cd");
        cleanup_cd_memory(current_pwd, old_pwd);
        return 1;
    }
    return 0;
}

// Belirtilen dizine git
static int change_to_directory(char *path, char *current_pwd, char *old_pwd)
{
    if (chdir(path) != 0)
    {
        perror("cd");
        cleanup_cd_memory(current_pwd, old_pwd);
        return 1;
    }
    return 0;
}

// PWD ve OLDPWD environment değişkenlerini güncelle
static void update_pwd_env(char *old_pwd)
{
    char *new_pwd;
    
    new_pwd = getcwd(NULL, 0);
    if (new_pwd)
    {
        setenv("PWD", new_pwd, 1);
        if (old_pwd)
            setenv("OLDPWD", old_pwd, 1);
        free(new_pwd);
    }
}

int built_cd(t_parser *cmd)
{
    char *current_pwd;
    char *old_pwd;
    int result;
    
    current_pwd = getcwd(NULL, 0);
    if (current_pwd)
        old_pwd = ft_strdup(current_pwd);
    else
        old_pwd = NULL;
    
    if (validate_cd_args(cmd))
    {
        cleanup_cd_memory(current_pwd, old_pwd);
        return 1;
    }
    
    if (cmd->argv[1] == NULL || cmd->argv[1][0] == '\0')
        result = change_to_home(current_pwd, old_pwd);
    else
        result = change_to_directory(cmd->argv[1], current_pwd, old_pwd);
    
    if (result == 0)
    {
        update_pwd_env(old_pwd);
        cleanup_cd_memory(current_pwd, old_pwd);
    }
    
    return result;
}

void builtin_pwd(void)
{
    char *cwd = getcwd(NULL, 0);
    if (cwd)
    {
        printf("%s\n", cwd);
		free(cwd);
    }
    else
        perror("pwd");
}

// exit argümanının geçerli sayı olup olmadığını kontrol et
static int is_valid_number(char *str)
{
    int i;
    
    if (!str)
        return 1;
    
    i = 0;
    if (str[0] == '-' || str[0] == '+')
        i = 1;
    
    if (!str[i])
        return 0;
    
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return 0;
        i++;
    }
    return 1;
}

// exit kodu hesapla (0-255 arasında normalize et)
static int calculate_exit_code(char *arg)
{
    int exit_code;
    
    if (!arg)
        return 0;
    
    exit_code = ft_atoi(arg);
    return ((exit_code % 256) + 256) % 256;
}

// exit için cleanup işlemleri
static void cleanup_and_exit(t_parser *cmd, int exit_code)
{
    close(cmd->fd_i);
    close(cmd->fd_o);
    gb_free_all();
    env_gb_free_all();
    exit(exit_code);
}

void builtin_exit(t_parser *cmd)
{
    int exit_code;
    
    if (cmd->argv[1] && cmd->argv[2])
    {
        printf("bash: exit: too many arguments\n");
        return;
    }
    
    if (!is_valid_number(cmd->argv[1]))
    {
        printf("bash: exit: %s: numeric argument required\n", cmd->argv[1]);
        cleanup_and_exit(cmd, 2);
    }
    
    exit_code = calculate_exit_code(cmd->argv[1]);
    cleanup_and_exit(cmd, exit_code);
}

void builtin_env(t_env *env_list)
{
    t_env *current = env_list;
    
    while (current)
    {
        if (current->value)
            printf("%s=%s\n", current->key, current->value);
        current = current->next;
    }
}
