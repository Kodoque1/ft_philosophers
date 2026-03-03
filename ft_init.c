/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_init.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/27 21:55:46 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/03 17:07:59 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

static void	init_cleanup_state(t_data *data)
{
	data->philosophers = NULL;
	data->forks = NULL;
	data->initialized_philos = 0;
	data->print_mutex_initialized = 0;
	data->death_mutex_initialized = 0;
	data->start_mutex_initialized = 0;
	data->all_philosophers_ready = 0;
	data->simulation_ended = 0;
	data->start_time = 0;
	data->monitor_started = 0;
}

static int	init_philo_at(t_data *data, int i)
{
	data->philosophers[i].id = i + 1;
	data->philosophers[i].times_eaten = 0;
	data->philosophers[i].data = data;
	data->philosophers[i].last_meal_time = 0;
	if (pthread_mutex_init(&data->forks[i], NULL) != 0)
		return (NOK);
	if (pthread_mutex_init(&data->philosophers[i].meal_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&data->forks[i]);
		return (NOK);
	}
	data->initialized_philos++;
	return (OK);
}

static int	init_philosophers_loop(t_data *data)
{
	int	i;

	data->philosophers = malloc(sizeof(t_philosopher) * data->num_philosophers);
	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philosophers);
	if (!data->philosophers || !data->forks)
		return (NOK);
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
	init_cleanup_state(data);
	if (parse_arguments(data, argv) == NOK)
		return (NOK);
	if (init_philosophers_loop(data) == NOK)
		return (NOK);
	if (pthread_mutex_init(&data->print_mutex, NULL) != 0)
		return (NOK);
	data->print_mutex_initialized = 1;
	if (pthread_mutex_init(&data->death_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&data->print_mutex);
		data->print_mutex_initialized = 0;
		return (NOK);
	}
	data->death_mutex_initialized = 1;
	if (pthread_mutex_init(&data->start_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&data->death_mutex);
		pthread_mutex_destroy(&data->print_mutex);
		data->death_mutex_initialized = 0;
		data->print_mutex_initialized = 0;
		return (NOK);
	}
	data->start_mutex_initialized = 1;
	return (OK);
}
