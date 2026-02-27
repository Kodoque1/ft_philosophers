/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philosophers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:46:37 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/27 21:38:05 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	start_philosophers(t_data *data, void *(*philosopher_routine)(void *))
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		pthread_create(&data->philosophers[i].thread, NULL, philosopher_routine,
			&data->philosophers[i]);
	}
	return (OK);
}

int	*philosophers(void *arg)
{
	t_philosopher	*philosopher;

	philosopher = (t_philosopher *)arg;
	while (1)
	{
		if (philosopher->times_eaten >= philosopher->data->num_times_must_eat)
			break ;
		if (philosopher->id % 2 == 0)
		{
			pthread_mutex_lock(&philosopher->data->forks[philosopher->id - 1
				% philosopher->data->num_philosophers]);
			pthread_mutex_lock(&philosopher->data->forks[philosopher->id
				% philosopher->data->num_philosophers]);
		}
		else
		{
			pthread_mutex_lock(&philosopher->data->forks[philosopher->id
				% philosopher->data->num_philosophers]);
			pthread_mutex_lock(&philosopher->data->forks[philosopher->id - 1
				% philosopher->data->num_philosophers]);
		}
	}
	return (NULL);
}



int	wait_for_philosophers(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		pthread_join(data->philosophers[i].thread, NULL);
	}
	return (OK);
}
