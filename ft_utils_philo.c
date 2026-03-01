/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils_philo.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:30:56 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 17:29:41 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	fragmented_sleep(int milliseconds)
{
	long long	start_time;
	long long	current_time;

	start_time = get_current_time();
	while (1)
	{
		current_time = get_current_time();
		if (current_time - start_time >= milliseconds)
			break ;
		usleep(100);
	}
	return (0);
}

int	concurent_print(const char *message, t_data *data)
{
	if (pthread_mutex_lock(&data->print_mutex) != 0)
		return (NOK);
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
	int		timestamp;

	timestamp = get_current_time() - data->start_time;
	if (timestamp == -1)
		return (NOK);
	len = ft_itoa(timestamp, buffer, sizeof(buffer));
	if (len == -1)
		return (NOK);
	buffer[len] = ' ';
	len += ft_itoa(id, buffer + len + 1, sizeof(buffer) - len - 1);
	if (len == -1)
		return (NOK);
	buffer[len + 1] = ' ';
	buffer[len + 2] = '\0';
	return (concurrent_print(buffer, data) || concurrent_print(message, data));
}
