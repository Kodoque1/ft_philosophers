/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:26:24 by zaddi             #+#    #+#             */
/*   Updated: 2026/02/27 22:01:44 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PHILO_H
# define FT_PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_philosopher
{
	int				id;
	int				times_eaten;
	pthread_t		thread;
	t_data			*data;
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
	int				someone_died;
}					t_data;

typedef struct s_monitor
{
	pthread_t		thread;
	t_data			*data;
}					t_monitor;

int					validate_args(int argc, char **argv);
int					ft_atoi(const char *str);
int					is_valid_number(const char *str);
int					init_data(t_data *data, char **argv);
int					start_philosophers(t_data *data,
						void *(*philosopher_routine)(void *));
int					wait_for_philosophers(t_data *data);
int					concurent_print(const char *message, t_data *data);
void				cleanup_data(t_data *data);
int					*philosophers(void *arg);
int					*monitoring_thread(void *arg);
int					get_current_time(void);
int					fragmented_sleep(int milliseconds);
int					ft_strlen(const char *str);

#endif FT_PHILO_H
