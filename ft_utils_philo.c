/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils_philo.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:30:56 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 18:31:49 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	fragmented_sleep(int milliseconds, t_data *data)
{
	long long	start_time;
	long long	current_time;

	start_time = get_current_time();
	while (1)
	{
		if (data->simulation_ended)
			return (OK);
		current_time = get_current_time();
		if (current_time - start_time >= milliseconds)
			break ;
		usleep(100);
	}
	return (OK);
}

int	concurent_print(const char *message, t_data *data)
{
	if (data->simulation_ended)
		return (NOK);
	if (pthread_mutex_lock(&data->print_mutex) != 0)
		return (NOK);
	if (data->simulation_ended)
		return (pthread_mutex_unlock(&data->print_mutex), NOK);
	write(STDOUT_FILENO, message, ft_strlen(message));
	write(STDOUT_FILENO, "\n", 1);
	pthread_mutex_unlock(&data->print_mutex);
	return (OK);
}

int	timestamp(t_data *data)
{
	int	current_time;

	current_time = get_current_time();
	if (current_time == -1)
		return (-1);
	return (current_time - data->start_time);
}

int	philo_print(int id, const char *message, t_data *data)
{
	int		len;
	char	buffer[INT_SIZE * 2 + 3];
	int		ts;

	if (data->simulation_ended)
		return (NOK);
	ts = get_current_time() - data->start_time;
	if (ts == -1)
		return (NOK);
	len = ft_itoa(ts, buffer, sizeof(buffer));
	if (len == -1)
		return (NOK);
	buffer[len] = ' ';
	len += ft_itoa(id, buffer + len + 1, (int)sizeof(buffer) - len - 1);
	if (len == -1)
		return (NOK);
	buffer[len + 1] = ' ';
	buffer[len + 2] = '\0';
	if (concurent_print(buffer, data) == NOK)
		return (NOK);
	return (concurent_print(message, data));
}
