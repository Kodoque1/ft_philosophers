/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:30:56 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/01 18:31:49 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_philo.h"

int	ft_atoi(const char *str)
{
	int	result;
	int	sign;
	int	i;

	result = 0;
	sign = 1;
	i = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result * sign);
}

int	ft_itoa(int n, char *buffer, int buffer_size)
{
	int		i;
	int		is_negative;
	char	temp;
	int		j;

	if (buffer_size < 2)
		return (-1);
	is_negative = (n < 0);
	if (is_negative)
		n = -n;
	i = 0;
	if (n == 0)
		buffer[i++] = '0';
	while (n > 0 && i < buffer_size - 1)
	{
		buffer[i++] = (n % 10) + '0';
		n /= 10;
	}
	if (is_negative && i < buffer_size - 1)
		buffer[i++] = '-';
	buffer[i] = '\0';
	j = 0;
	while (j < i / 2)
	{
		temp = buffer[j];
		buffer[j] = buffer[i - j - 1];
		buffer[i - j - 1] = temp;
		j++;
	}
	return (i);
}

int	is_valid_number(const char *str)
{
	int	i;

	i = 0;
	if (str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	get_current_time(void)
{
	struct timeval	tv;
	long long		milliseconds;

	if (gettimeofday(&tv, NULL) == -1)
		return (-1);
	milliseconds = (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL);
	return ((int)milliseconds);
}

int	ft_strlen(const char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}
