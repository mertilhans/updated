/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_basic.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: merilhan <merilhan@42kocaeli.com.tr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 00:00:00 by mertilhan13       #+#    #+#             */
/*   Updated: 2025/08/01 02:17:36 by merilhan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shell.h"
#include <stdlib.h>
#include <string.h>

t_env	*find_env(t_env *env_list, const char *key)
{
	t_env	*current;

	if (!env_list || !key)
		return (NULL);
	current = env_list;
	while (current)
	{
		if (ft_strcmp(current->key, key) == 0)
			return (current);
		current = current->next;
	}
	return (NULL);
}

char	*get_env_value(t_env *env_list, char *key)
{
	t_env	*env_variable;

	env_variable = find_env(env_list, key);
	if (env_variable)
		return (env_variable->value);
	return (NULL);
}
