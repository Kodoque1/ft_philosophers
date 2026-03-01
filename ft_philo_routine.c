/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo_routine.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/01 17:00:00 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 18:31:49 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	philo_eat(t_philosopher *philo)
{
	int	current_time;

	if (philo_print(philo->id, "is eating", philo->data) == NOK)
		return (end_simulation(philo->data), NOK);
	current_time = get_current_time();
	if (current_time == -1)
		return (concurent_print("Error: Failed to get current time.",
				philo->data), end_simulation(philo->data), NOK);
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = current_time;
	philo->times_eaten++;
	pthread_mutex_unlock(&philo->meal_mutex);
	fragmented_sleep(philo->data->time_to_eat, philo->data);
	if (is_sim_ended(philo->data))
		return (NOK);
	return (OK);
}

static int	philo_think(t_philosopher *philo)
{
	if (philo_print(philo->id, "is thinking", philo->data) == NOK)
		return (end_simulation(philo->data), NOK);
	return (OK);
}

static int	philo_sleep(t_philosopher *philo)
{
	if (philo_print(philo->id, "is sleeping", philo->data) == NOK)
		return (end_simulation(philo->data), NOK);
	fragmented_sleep(philo->data->time_to_sleep, philo->data);
	if (is_sim_ended(philo->data))
		return (NOK);
	return (OK);
}

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

void	*philosophers(void *arg)
{
	t_philosopher	*philo;
	int				first;
	int				second;

	philo = (t_philosopher *)arg;
	if (philo->data->num_philosophers > 1 && philo->id % 2 == 0)
		usleep(philo->data->time_to_eat * 1000);
	while (1)
	{
		if (philo->data->num_times_must_eat != -1
			&& philo->times_eaten >= philo->data->num_times_must_eat)
			break ;
		if (philo_think(philo) == NOK)
			return (NULL);
		if (is_sim_ended(philo->data))
			return (NULL);
		if (philo->data->num_philosophers == 1)
		{
			while (!is_sim_ended(philo->data))
				usleep(1000);
			return (NULL);
		}
		get_fork_indices(philo, &first, &second);
		if (acquire_forks(philo, first, second) == NOK)
			return (NULL);
		if (philo_eat(philo) == NOK)
			return (release_forks(philo), NULL);
		release_forks(philo);
		if (philo_sleep(philo) == NOK)
			return (NULL);
	}
	return (NULL);
}
