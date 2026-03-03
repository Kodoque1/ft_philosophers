/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo_routine.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/01 17:00:00 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/03 11:37:48 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	philo_eat(t_philosopher *philo)
{
	int	current_time;

	current_time = get_current_time();
	if (current_time == -1)
		return (concurent_print("Error: Failed to get current time.",
				philo->data), end_simulation(philo->data), NOK);
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_time = current_time;
	philo->times_eaten++;
	pthread_mutex_unlock(&philo->meal_mutex);
	if (philo_print(philo->id, "is eating", philo->data) == NOK)
		return (end_simulation(philo->data), NOK);
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

static int	philo_cycle(t_philosopher *philo)
{
	if (philo_think(philo) == NOK)
		return (NOK);
	if (is_sim_ended(philo->data))
		return (NOK);
	if (philo->data->num_philosophers == 1)
	{
		while (!is_sim_ended(philo->data))
			usleep(1000);
		return (NOK);
	}
	if (acquire_forks(philo) == NOK)
		return (NOK);
	if (philo_eat(philo) == NOK)
		return (release_forks(philo), NOK);
	release_forks(philo);
	if (philo_sleep(philo) == NOK)
		return (NOK);
	if (philo->data->num_philosophers % 2 != 0)
		if (fragmented_sleep(philo->data->time_to_eat / 2, philo->data) == NOK)
			return (NOK);
	return (OK);
}

void	*philosophers(void *arg)
{
	t_philosopher	*philo;
	int				times_eaten;

	philo = (t_philosopher *)arg;
	wait_for_start(philo);
	while (1)
	{
		times_eaten = get_times_eaten(philo);
		if (philo->data->num_times_must_eat != -1
			&& times_eaten >= philo->data->num_times_must_eat)
			break ;
		if (philo_cycle(philo) == NOK)
			return (NULL);
	}
	return (NULL);
}
