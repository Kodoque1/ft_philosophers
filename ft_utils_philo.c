/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils_philo.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:30:56 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 23:39:20 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	is_sim_ended(t_data *data)
{
	int	ended;

	pthread_mutex_lock(&data->death_mutex);
	ended = data->simulation_ended;
	pthread_mutex_unlock(&data->death_mutex);
	return (ended);
}

void	end_simulation(t_data *data)
{
	pthread_mutex_lock(&data->death_mutex);
	data->simulation_ended = 1;
	pthread_mutex_unlock(&data->death_mutex);
}

int	fragmented_sleep(int milliseconds, t_data *data)
{
	long long	start_time;
	long long	current_time;

	start_time = get_current_time();
	while (1)
	{
		if (is_sim_ended(data))
			return (OK);
		current_time = get_current_time();
		if (current_time - start_time >= milliseconds)
			break ;
		usleep(100);
	}
	return (OK);
}


