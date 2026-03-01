/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_monitoring.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/27 21:38:20 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 18:31:49 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

static int	check_philosopher_death(t_data *data);
static int	check_all_philosophers_full(t_data *data);

int	start_monitoring_thread(t_data *data, void *(*monitoring_routine)(void *))
{
	pthread_t	monitoring_thread;

	if (pthread_create(&monitoring_thread, NULL, monitoring_routine, data) != 0)
		return (NOK);
	if (pthread_detach(monitoring_thread) != 0)
	{
		end_simulation(data);
		pthread_join(monitoring_thread, NULL);
		return (NOK);
	}
	return (OK);
}

void	*monitoring_thread(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (1)
	{
		if (check_philosopher_death(data) || (data->num_times_must_eat != -1
				&& check_all_philosophers_full(data)))
		{
			end_simulation(data);
			return (NULL);
		}
		if (usleep(1000) == -1)
		{
			end_simulation(data);
			return (NULL);
		}
	}
	return (NULL);
}

static int	check_all_philosophers_full(t_data *data)
{
	int	i;
	int	times_eaten;

	i = 0;
	while (i < data->num_philosophers)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_mutex);
		times_eaten = data->philosophers[i].times_eaten;
		pthread_mutex_unlock(&data->philosophers[i].meal_mutex);
		if (times_eaten < data->num_times_must_eat)
			return (0);
		i++;
	}
	return (1);
}

static int	check_philosopher_death(t_data *data)
{
	int	i;
	int	current_time;
	int	last_meal;

	i = 0;
	current_time = get_current_time();
	if (current_time == -1)
		return (1);
	while (i < data->num_philosophers)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_mutex);
		last_meal = data->philosophers[i].last_meal_time;
		pthread_mutex_unlock(&data->philosophers[i].meal_mutex);
		if (current_time - last_meal >= data->time_to_die)
		{
			philo_print(i + 1, "died", data);
			return (1);
		}
		i++;
	}
	return (0);
}
