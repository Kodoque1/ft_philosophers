/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_monitoring.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/27 21:38:20 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/27 22:11:40 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	*monitoring_thread(void *arg)
{
	t_data	*data;
	int		i;
	int		all_full;

	data = (t_data *)arg;
	i = 0;
	while (1)
	{
		while (i < data->num_philosophers)
		{
			if (get_current_time()
				- data->philosophers[i].last_meal_time > data->time_to_die)
			{
				concurent_print("A philosopher has died.", data);
				data->someone_died = 1;
				return (NULL);
			}
		}
		if (data->num_times_must_eat > 0)
		{
			all_full = 1;
			i = 0;
			while (i < data->num_philosophers)
			{
				if (data->philosophers[i].times_eaten < data->num_times_must_eat)
				{
					all_full = 0;
					break ;
				}
				i++;
			}
			if (all_full)
				return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
