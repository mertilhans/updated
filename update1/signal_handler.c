/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal_handler.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: merilhan <merilhan@42kocaeli.com.tr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 00:00:00 by mertilhan13       #+#    #+#             */
/*   Updated: 2025/08/06 00:14:06 by merilhan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shell.h"
#include <signal.h>

void handle_sigint(int signum)
{
    (void)signum;
    
    printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
void    handle_sigint_heredoc(int signum)
{
    (void)signum;
    printf("\n");
    // Interactive signals'e geri dön exit etme!
    setup_interactive_signals();
    // Heredoc'u iptal et ama programı sonlandırma
}
void    heredoc_signals(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;
    
    sa_int.sa_handler = handle_sigint_heredoc;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}
void setup_interactive_signals(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;
    
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}
void setup_parent_execution_signals(void)
{
    struct sigaction sa_int, sa_quit;
    
    sa_int.sa_handler = SIG_IGN;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}
void setup_child_signals(void)
{
    signal(SIGINT, SIG_DFL); 
    signal(SIGQUIT, SIG_DFL); 
}

int calculate_exit_status(int status)
{
    if (WIFEXITED(status))
        return (WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        if (sig == SIGINT)
            return (130);
        else if (sig == SIGQUIT)
            return (131);
        else
            return (128 + sig);
    }
    return (1);
}