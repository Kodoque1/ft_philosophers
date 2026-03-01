/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo_routine.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/01 17:00:00 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 17:30:09 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	philo_eat(t_philosopher *philo)
{
	if (concurent_print("is eating", philo->data) == NOK)
		return (philo->data->simulation_ended = 1, NOK);
	philo->last_meal_time = get_current_time();
	if (philo->last_meal_time == -1)
	{
		concurent_print("Error: Failed to get current time.", philo->data);
		return (philo->data->simulation_ended = 1, NOK);
	}
	philo->times_eaten++;
	fragmented_sleep(philo->data->time_to_eat);
	return (OK);
}

int	*philosophers(void *arg)
{
	t_philosopher	*philo;
	int				first;
	int				second;

	philo = (t_philosopher *)arg;
	while (1)
	{
		if (philo->times_eaten >= philo->data->num_times_must_eat)
			break ;
		if (concurent_print("is thinking", philo->data) == NOK)
			return (philo->data->simulation_ended = 1, NULL);

		first = philo->id - 1 % philo->data->num_philosophers;
		second = philo->id % philo->data->num_philosophers;
		if (philo->id % 2 != 0)
		{
			first = philo->id;
			second = philo->id - 1 % philo->data->num_philosophers;
		}
		if (acquire_forks(philo, first, second) == NOK)
			return (NULL);
		if (philo_eat(philo) == NOK)
			return (release_forks(philo), NULL);
		release_forks(philo);
		if (concurent_print("is sleeping", philo->data) == NOK)
			return (philo->data->simulation_ended = 1, NULL);
		fragmented_sleep(philo->data->time_to_sleep);
	}
	return (NULL);
}
