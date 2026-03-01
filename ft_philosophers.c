/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philosophers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:46:37 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 18:31:49 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	start_philosophers(t_data *data, void *(*philosopher_routine)(void *))
{
	int	i;

	i = 0;
	data->start_time = get_current_time();
	if (data->start_time == -1)
		return (NOK);
	while (i < data->num_philosophers)
	{
		data->philosophers[i].last_meal_time = data->start_time;
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosopher_routine, &data->philosophers[i]) != 0)
		{
			end_simulation(data);
			while (--i >= 0)
				pthread_join(data->philosophers[i].thread, NULL);
			return (NOK);
		}
		i++;
	}
	return (OK);
}

int	wait_for_philosophers(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		if (pthread_join(data->philosophers[i].thread, NULL) != 0)
			return (NOK);
		i++;
	}
	return (OK);
}
