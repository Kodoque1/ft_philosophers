/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils_philo.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:30:56 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/27 21:51:39 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	fragmented_sleep(int milliseconds)
{
	long long	start_time;
	long long	current_time;

	start_time = get_current_time();
	while (1)
	{
		current_time = get_current_time();
		if (current_time - start_time >= milliseconds)
			break ;
		usleep(100);
	}
	return (0);
}

int	concurent_print(const char *message, t_data *data)
{
	pthread_mutex_lock(&data->print_mutex);
	write(STDOUT_FILENO, message, ft_strlen(message));
	write(STDOUT_FILENO, "\n", 1);
	pthread_mutex_unlock(&data->print_mutex);
	return (0);
}
