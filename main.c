/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:17:26 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 16:39:18 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	validate_args(int argc, char **argv)
{
	int	i;

	if (!(argc == 5 || argc == 6))
	{
		write(STDERR_FILENO, "Error: Invalid number of arguments.\n", 34);
		return (-1);
	}
	while (i < argc)
	{
		if (!is_valid_number(argv[i]))
		{
			write(STDERR_FILENO,
				"Error: Arguments must be positive integers.\n", 43);
			return (NOK);
		}
		i++;
	}
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
	int		i;

	if (validate_args(argc, argv) == OK && init_data(&data, argv) == OK)
	{
		if (start_philosophers(&data, data.philosophers) == NOK
			|| monitoring_thread(&data) == NOK
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
