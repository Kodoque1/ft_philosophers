/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo_forks.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/01 17:00:00 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 23:39:23 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

static void	get_fork_indices(t_philosopher *philo, int *first, int *second)
{
	*first = (philo->id - 1) % philo->data->num_philosophers;
	*second = philo->id % philo->data->num_philosophers;
	if (philo->id % 2 != 0)
	{
		*first = philo->id % philo->data->num_philosophers;
		*second = (philo->id - 1) % philo->data->num_philosophers;
	}
}

int	lock_fork(pthread_mutex_t *fork, t_philosopher *philo, t_data *data)
{
	if (pthread_mutex_lock(fork) != 0)
	{
		concurent_print("Error: Failed to lock fork mutex.", data);
		end_simulation(data);
		return (NOK);
	}
	if (philo_print(philo->id, "has taken a fork", data) == NOK)
	{
		pthread_mutex_unlock(fork);
		return (NOK);
	}
	return (OK);
}

int	unlock_fork(pthread_mutex_t *fork, t_data *data)
{
	if (pthread_mutex_unlock(fork) != 0)
	{
		concurent_print("Error: Failed to unlock fork mutex.", data);
		end_simulation(data);
		return (NOK);
	}
	return (OK);
}

int	acquire_forks(t_philosopher *philo)
{
	int	first;
	int	second;

	get_fork_indices(philo, &first, &second);
	if (lock_fork(&philo->data->forks[first], philo, philo->data) == NOK)
		return (NOK);
	if (lock_fork(&philo->data->forks[second], philo, philo->data) == NOK)
	{
		pthread_mutex_unlock(&philo->data->forks[first]);
		return (NOK);
	}
	return (OK);
}

int	release_forks(t_philosopher *philo)
{
	if (unlock_fork(&philo->data->forks[(philo->id - 1)
				% philo->data->num_philosophers], philo->data) == NOK)
		return (NOK);
	if (unlock_fork(&philo->data->forks[philo->id
				% philo->data->num_philosophers], philo->data) == NOK)
		return (NOK);
	return (OK);
}
