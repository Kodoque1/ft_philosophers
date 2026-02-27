/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_init.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/27 21:55:46 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/27 22:01:44 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	init_data(t_data *data, char **argv)
{
	int	i;

	if (parse_arguments(data, argv) == NOK)
		return (NOK);
	i = 0;
	data->philosophers = malloc(sizeof(t_philosopher) * data->num_philosophers);
	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philosophers);
	if (!data->philosophers || !data->forks)
		return (NOK);
	while (i < data->num_philosophers)
	{
		data->philosophers[i].id = i + 1;
		data->philosophers[i].times_eaten = 0;
		if (pthread_mutex_init(&data->forks[i], NULL) != 0)
		{
			cleanup_data(data);
			return (NOK);
		}
	}
	if (pthread_mutex_init(&data->print_mutex, NULL) != 0)
	{
		cleanup_data(data);
		return (NOK);
	}
	return (OK);
}

int	parse_arguments(t_data *data, char **argv)
{
	data->num_philosophers = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argv[5])
		data->num_times_must_eat = ft_atoi(argv[5]);
	else
		data->num_times_must_eat = -1;
	if (data->num_philosophers <= 0 || data->time_to_die <= 0
		|| data->time_to_eat <= 0 || data->time_to_sleep <= 0 || (argv[5]
			&& data->num_times_must_eat <= 0))
		return (NOK);
	return (OK);
}
