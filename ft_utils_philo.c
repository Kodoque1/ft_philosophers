/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils_philo.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:30:56 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/03 11:26:58 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	is_sim_ended(t_data *data)
{
	int	ended;

	pthread_mutex_lock(&data->death_mutex);
	ended = data->simulation_ended;
	pthread_mutex_unlock(&data->death_mutex);
	return (ended);
}

void	end_simulation(t_data *data)
{
	pthread_mutex_lock(&data->death_mutex);
	data->simulation_ended = 1;
	pthread_mutex_unlock(&data->death_mutex);
}

int	get_times_eaten(t_philosopher *philo)
{
	int	times_eaten;

	pthread_mutex_lock(&philo->meal_mutex);
	times_eaten = philo->times_eaten;
	pthread_mutex_unlock(&philo->meal_mutex);
	return (times_eaten);
}

void	wait_for_start(t_philosopher *philo)
{
	int	ready;

	ready = 0;
	while (!ready)
	{
		pthread_mutex_lock(&philo->data->start_mutex);
		ready = philo->data->all_philosophers_ready;
		pthread_mutex_unlock(&philo->data->start_mutex);
		if (!ready)
			usleep(100);
	}
	if (philo->data->num_philosophers > 1 && philo->id % 2 == 0)
		fragmented_sleep(philo->data->time_to_eat, philo->data);
}

int	fragmented_sleep(int milliseconds, t_data *data)
{
	long long	start_time;
	long long	current_time;

	start_time = get_current_time();
	while (1)
	{
		if (is_sim_ended(data))
			return (OK);
		current_time = get_current_time();
		if (current_time - start_time >= milliseconds)
			break ;
		usleep(100);
	}
	return (OK);
}



