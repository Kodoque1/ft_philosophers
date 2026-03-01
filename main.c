/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:17:26 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 23:51:23 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	ft_strlen(const char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

int	validate_args(int argc, char **argv)
{
	int	i;

	if (!(argc == 5 || argc == 6))
	{
		write(STDERR_FILENO, "Error: Invalid number of arguments.\n", 37);
		return (NOK);
	}
	i = 1;
	while (i < argc)
	{
		if (!is_valid_number(argv[i]))
		{
			write(STDERR_FILENO,
				"Error: Arguments must be positive integers.\n", 45);
			return (NOK);
		}
		i++;
	}
	return (OK);
}

static void	cleanup_philo_mutexes(t_data *data)
{
	int	i;

	i = 0;
	while (data->forks && data->philosophers && i < data->initialized_philos)
	{
		pthread_mutex_destroy(&data->forks[i]);
		pthread_mutex_destroy(&data->philosophers[i].meal_mutex);
		i++;
	}
}

void	cleanup_data(t_data *data)
{
	cleanup_philo_mutexes(data);
	if (data->print_mutex_initialized)
		pthread_mutex_destroy(&data->print_mutex);
	if (data->death_mutex_initialized)
		pthread_mutex_destroy(&data->death_mutex);
	free(data->philosophers);
	free(data->forks);
	data->philosophers = NULL;
	data->forks = NULL;
	data->initialized_philos = 0;
	data->print_mutex_initialized = 0;
	data->death_mutex_initialized = 0;
}

int	main(int argc, char **argv)
{
	t_data	data;

	data = (t_data){0};

	if (validate_args(argc, argv) == OK && init_data(&data, argv) == OK)
	{
		if (start_philosophers(&data, philosophers) == NOK
			|| start_monitoring_thread(&data, monitoring_thread) == NOK
			|| wait_for_philosophers(&data) == NOK)
		{
			cleanup_data(&data);
			return (-1);
		}
	}
	else
		return (-1);
	cleanup_data(&data);
	return (0);
}
