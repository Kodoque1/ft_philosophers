/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:26:24 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/26 18:05:27 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PHILO_H
# define FT_PHILO_H

# include <pthread.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <sys/time.h>

typedef struct s_philosopher
{
	int				id;
	int				times_eaten;
	pthread_t		thread;
}					t_philosopher;

typedef enum return_code
{
	OK,
	NOK
}					t_return_code;

typedef struct s_data
{
	int				num_philosophers;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				num_times_must_eat;
	t_philosopher	*philosophers;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_mutex;
}					t_data;

int					validate_args(int argc, char **argv);
int					atoi(const char *str);
int					is_valid_number(const char *str);
int					init_data(t_data *data, char **argv);
int					start_philosophers(t_data *data,
						void *(*philosopher_routine)(void *));
int					wait_for_philosophers(t_data *data);
int					concurent_print(const char *message, t_data *data);
void				cleanup_data(t_data *data);

#endif FT_PHILO_H
