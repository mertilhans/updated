/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: husarpka <husarpka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 00:23:50 by husarpka          #+#    #+#             */
/*   Updated: 2025/08/04 00:24:58 by husarpka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// abi burda değişken karakterini kontrol ediyoruz
int is_valid_char(char c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_');
}

// stringi expand ettikten sonra boşluklara göre böl
char **split_expanded_string(char *str)
{
    char **result = gb_malloc(sizeof(char*) * 10);
    int count = 0;
    int i = 0, start;
    
    if (!str || !result)
        return NULL;
        
    while (str[i])
    {
        // boşlukları skip et abi
        while (str[i] == ' ' || str[i] == '\t')
            i++;
        if (!str[i])
            break;
            
        start = i;
        // kelimeyi bulana kadar devam et
        while (str[i] && str[i] != ' ' && str[i] != '\t')
            i++;
            
        result[count] = gb_malloc(i - start + 1);
        ft_memcpy(result[count], str + start, i - start);
        result[count][i - start] = '\0';
        count++;
    }
    result[count] = NULL;
    return result;
}

// ana expansion fonksiyonu abi - $VAR şeylerini çözüyor
char *expand_variables(char *str, t_env *env_list, int exit_status)
{
    char *res;
    int res_len = 0;
    int res_cap = 1024;
    int i = 0;
    if (!str)
        return NULL;
    res = gb_malloc(res_cap);
    if (!res)
        return NULL;
    res[0] = '\0';
    while (str[i])
    {
        if (str[i] == '$')
        {
            char *expanded = NULL;
            // $ işaretini gördük, değişkeni expand et
            expanded = handle_dollar(str, &i, env_list, exit_status);
            if (expanded)
            {
                int add_len = ft_strlen(expanded);
                if (res_len + add_len >= res_cap)
                {
                    res_cap *= 2;
                    char *new_res = realloc(res, res_cap);
                    if (!new_res)
                    {
                        gc_free(res);
                        gc_free(expanded);
                        return NULL;
                    }
                    res = new_res;
                }
                ft_memcpy(res + res_len, expanded, add_len);
                res_len += add_len;
                res[res_len] = '\0';
                gc_free(expanded);
            }
        }
        else
        {
            // Normal karakter - kaçış operatörü işlemi yok
            if (res_len + 1 >= res_cap)
            {
                res_cap *= 2;
                char *new_res = realloc(res, res_cap);
                if (!new_res)
                {
                    gc_free(res);
                    return NULL;
                }
                res = new_res;
            }
            res[res_len++] = str[i++];
            res[res_len] = '\0';
        }
    }
    return res;
}

char *handle_dollar(char *str, int *i, t_env *env_list, int exit_status)
{
    char *var_name;
    char *expanded = NULL;
    int start_i;
    (*i)++; // $ karakterini atla
    if (!str[*i])
        return ft_strdup("$");
    // $? 
    if (str[*i] == '?')
    {
        (*i)++;
        return ft_itoa(exit_status);
    }
    // $$
    else if (str[*i] == '$')
    {
        (*i)++;
        return ft_itoa(getpid());
    }
    // $VAR_NAME
    else if (isalpha(str[*i]) || str[*i] == '_')
    {
        start_i = *i;
        while (str[*i] && (isalnum(str[*i]) || str[*i] == '_'))
            (*i)++;
        var_name = gb_malloc((*i - start_i + 1));
        if (!var_name)
            return NULL;
        ft_memcpy(var_name, str + start_i, (*i - start_i));
        var_name[*i - start_i] = '\0';
        
        t_env *env_node = find_env(env_list, var_name);
        if (env_node && env_node->value)
        {
            expanded = ft_strdup(env_node->value);
        }
        else
        {
            char *export_value = find_export_value(var_name);
            if (export_value)
                expanded = ft_strdup(export_value);
            else
                expanded = ft_strdup(""); // Boş string döndür
        }
        
        gc_free(var_name);
        return expanded;
    }
    else
    {
        // Geçersiz $ kullanımı, $ karakterini normal karakter olarak dön
        return ft_strdup("$");
    }
}

char *handle_simple(const char *str, int *i, t_env *env_list)
{
    int len = 0;
    char *var_name;
    
    (*i)++;
    while (is_valid_char(str[*i + len]))
        len++;
    
    if (len == 0)
    {
        (*i)++;
        return ft_strdup("");
    }
    
    var_name = strndup(str + *i, len);
    *i += len;
    
    t_env *env_var = find_env(env_list, var_name);
    
    if (env_var == NULL)
    {
        char *result = ft_strdup("");
        gc_free(var_name);
        return result;
    }
    
    gc_free(var_name);
    
    if (env_var->value == NULL)
        return ft_strdup("");
    else
        return ft_strdup(env_var->value);
}

char *push_char_res(char *res, char c, int *res_len, int *res_cap)
{
    if (*res_len + 1 >= *res_cap)
    {
        *res_cap *= 2;
        char *new_res = realloc(res, *res_cap);
        if (!new_res)
            return res;
        res = new_res;
    }
    res[(*res_len)++] = c;
    res[*res_len] = '\0';
    return res;
}

static char *append_string_res(char *result, char *str, int *res_len, int *res_cap)
{
    int add_len = strlen(str);
    if (*res_len + add_len >= *res_cap)
    {
        *res_cap = (*res_len + add_len) * 2;
        char *new_res = realloc(result, *res_cap);
        if (!new_res)
            return result;
        result = new_res;
    }
    memcpy(result + *res_len, str, add_len);
    *res_len += add_len;
    result[*res_len] = '\0';
    return result;
}

char *expand_with_quotes(char *str, t_env *env_list, int exit_status)
{
    if (!str)
        return NULL;
    int str_len = ft_strlen(str);
    int res_len = 0, res_cap = str_len * 2, i = 0; // Capacity based on input length
    if (res_cap < 64)
        res_cap = 64; // Minimum capacity
    char quote_char = 0;
    char *result = gb_malloc(res_cap);
    if (!result)
        return NULL;
    result[0] = '\0';
    
    while (str[i])
    {
        if (str[i] == '\'' || str[i] == '"')
        {
            char current_quote = str[i];
            if (quote_char == 0)
            {
                // Quote başlangıcı - quote karakterini atla
                quote_char = current_quote;
                i++;
            }
            else if (quote_char == current_quote)
            {
                // Quote bitişi  quote karakterini atla
                quote_char = 0;
                i++;
            }
            else
            {
                // Farklı quote içindeyken - karakteri ekle
                result = push_char_res(result, str[i++], &res_len, &res_cap);
            }
        }
        else if (str[i] == '$' && quote_char != '\'')
        {
            // Variable expansion (single quote içinde değilse)
            char *expanded = handle_dollar(str, &i, env_list, exit_status);
            if (expanded)
            {
                result = append_string_res(result, expanded, &res_len, &res_cap);
                gc_free(expanded);
            }
        }
        else
        {
            // Normal karakter - kaçış operatörü işlemi yok
            result = push_char_res(result, str[i++], &res_len, &res_cap);
        }
    }
    
    return result;
}
