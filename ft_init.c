/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_init.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/27 21:55:46 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 23:39:14 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

static void	cleanup_init(t_data *data, int n)
{
	while (--n >= 0)
	{
		pthread_mutex_destroy(&data->forks[n]);
		pthread_mutex_destroy(&data->philosophers[n].meal_mutex);
	}
	free(data->philosophers);
	free(data->forks);
}

static int	init_philo_at(t_data *data, int i)
{
	data->philosophers[i].id = i + 1;
	data->philosophers[i].times_eaten = 0;
	data->philosophers[i].data = data;
	data->philosophers[i].last_meal_time = 0;
	if (pthread_mutex_init(&data->forks[i], NULL) != 0)
	{
		cleanup_init(data, i);
		return (NOK);
	}
	if (pthread_mutex_init(&data->philosophers[i].meal_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&data->forks[i]);
		cleanup_init(data, i);
		return (NOK);
	}
	return (OK);
}

static int	init_philosophers_loop(t_data *data)
{
	int	i;

	data->philosophers = malloc(sizeof(t_philosopher) * data->num_philosophers);
	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philosophers);
	if (!data->philosophers || !data->forks)
	{
		free(data->philosophers);
		free(data->forks);
		return (NOK);
	}
	i = 0;
	while (i < data->num_philosophers)
	{
		if (init_philo_at(data, i) == NOK)
			return (NOK);
		i++;
	}
	return (OK);
}

static int	parse_arguments(t_data *data, char **argv)
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

int	init_data(t_data *data, char **argv)
{
	if (parse_arguments(data, argv) == NOK)
		return (NOK);
	if (init_philosophers_loop(data) == NOK)
		return (NOK);
	if (pthread_mutex_init(&data->print_mutex, NULL) != 0)
	{
		cleanup_init(data, data->num_philosophers);
		return (NOK);
	}
	if (pthread_mutex_init(&data->death_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&data->print_mutex);
		cleanup_init(data, data->num_philosophers);
		return (NOK);
	}
	data->simulation_ended = 0;
	return (OK);
}
