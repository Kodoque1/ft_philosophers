/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:17:26 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/26 22:07:42 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	validate_args(int argc, char **argv)
{
	int	i;

	if (argc != 5 && argc != 6)
	{
		printf("Error: Invalid number of arguments.\n");
		return (-1);
	}
	while (i < argc)
	{
		if (!is_valid_number(argv[i]))
		{
			printf("Error: Arguments must be positive integers.\n");
			return (NOK);
		}
		i++;
	}
	return (OK);
}

int	init_data(t_data *data, char **argv)
{
	int	i;

	i = 0;
	data->num_philosophers = atoi(argv[1]);
	data->time_to_die = atoi(argv[2]);
	data->time_to_eat = atoi(argv[3]);
	data->time_to_sleep = atoi(argv[4]);
	if (argv[5])
		data->num_times_must_eat = atoi(argv[5]);
	else
		data->num_times_must_eat = -1;
	data->philosophers = malloc(sizeof(t_philosopher) * data->num_philosophers);
	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philosophers);
	if (!data->philosophers || !data->forks)
		return (NOK);
	while (i < data->num_philosophers)
	{
		data->philosophers[i].id = i + 1;
		data->philosophers[i].times_eaten = 0;
		pthread_mutex_init(&data->forks[i], NULL);
	}
	pthread_mutex_init(&data->print_mutex, NULL);
	return (OK);
}

void	cleanup_data(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		pthread_mutex_destroy(&data->forks[i]);
		i++;
	}
	pthread_mutex_destroy(&data->print_mutex);
	free(data->philosophers);
	free(data->forks);
}

int	main(int argc, char **argv)
{
	t_data	data;

	if (validate_args(argc, argv) == OK)
	{
		if (init_data(&data, argv) == OK)
		{
			// Start philosopher threads
			for (int i = 0; i < data.num_philosophers; i++)
			{
				pthread_create(&data.philosophers[i].thread, NULL,
					philosopher_routine, &data.philosophers[i]);
			}
			// Wait for all philosophers to finish
			for (int i = 0; i < data.num_philosophers; i++)
			{
				pthread_join(data.philosophers[i].thread, NULL);
			}
		}
		else
		{
			return (-1);
		}
	}
	else
		return (-1);
	cleanup_data(&data);
	return (0);
}
