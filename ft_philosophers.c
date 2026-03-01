/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philosophers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:46:37 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/02 00:13:31 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

static int	make_print_prefix(int id, t_data *data, char *buffer)
{
	int	ts;
	int	len;

	ts = get_current_time() - data->start_time;
	if (ts == -1)
		return (-1);
	len = ft_itoa(ts, buffer, INT_SIZE * 2 + 3);
	if (len == -1)
		return (-1);
	buffer[len++] = ' ';
	len += ft_itoa(id, buffer + len, INT_SIZE * 2 + 3 - len);
	buffer[len++] = ' ';
	buffer[len] = '\0';
	return (len);
}

int	concurent_print(const char *message, t_data *data)
{
	if (is_sim_ended(data))
		return (NOK);
	if (pthread_mutex_lock(&data->print_mutex) != 0)
		return (NOK);
	if (is_sim_ended(data))
		return (pthread_mutex_unlock(&data->print_mutex), NOK);
	write(STDOUT_FILENO, message, ft_strlen(message));
	write(STDOUT_FILENO, "\n", 1);
	pthread_mutex_unlock(&data->print_mutex);
	return (OK);
}

int	philo_print(int id, const char *message, t_data *data)
{
	char	buffer[INT_SIZE * 2 + 3];
	int		len;

	if (is_sim_ended(data))
		return (NOK);
	len = make_print_prefix(id, data, buffer);
	if (len == -1)
		return (NOK);
	if (pthread_mutex_lock(&data->print_mutex) != 0)
		return (NOK);
	if (is_sim_ended(data))
		return (pthread_mutex_unlock(&data->print_mutex), NOK);
	write(STDOUT_FILENO, buffer, len);
	write(STDOUT_FILENO, message, ft_strlen(message));
	write(STDOUT_FILENO, "\n", 1);
	pthread_mutex_unlock(&data->print_mutex);
	return (OK);
}

int	start_philosophers(t_data *data)
{
	int	i;

	i = 0;
	data->start_time = get_current_time();
	if (data->start_time == -1)
		return (NOK);
	while (i < data->num_philosophers)
	{
		data->philosophers[i].last_meal_time = data->start_time;
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosophers, &data->philosophers[i]) != 0)
		{
			end_simulation(data);
			while (--i >= 0)
				pthread_join(data->philosophers[i].thread, NULL);
			return (NOK);
		}
		i++;
	}
	return (OK);
}

int	wait_for_philosophers(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->num_philosophers)
	{
		if (pthread_join(data->philosophers[i].thread, NULL) != 0)
			return (NOK);
		i++;
	}
	return (OK);
}
