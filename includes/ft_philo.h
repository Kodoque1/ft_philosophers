/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_philo.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 17:26:24 by zaddi             #+#    #+#             */
/*   Updated: 2026/03/02 00:13:31 by zaddi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PHILO_H
# define FT_PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

#define INT_SIZE 12

typedef enum return_code
{
	OK,
	NOK
}					t_return_code;

typedef struct s_data	t_data;

typedef struct s_philosopher
{
	int				id;
	int				times_eaten;
	int				last_meal_time;
	pthread_mutex_t	meal_mutex;
	pthread_t		thread;
	t_data			*data;
}					t_philosopher;

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
	pthread_mutex_t	death_mutex;
	int				initialized_philos;
	int				print_mutex_initialized;
	int				death_mutex_initialized;
	int				simulation_ended;
	int				start_time;
}					t_data;

typedef struct s_monitor
{
	pthread_t		thread;
	t_data			*data;
}					t_monitor;

int					is_sim_ended(t_data *data);
void				end_simulation(t_data *data);
int					validate_args(int argc, char **argv);
int					ft_atoi(const char *str);
int					ft_itoa(int n, char *buffer, int buffer_size);
int					is_valid_number(const char *str);
int					init_data(t_data *data, char **argv);
int					start_philosophers(t_data *data);
int					wait_for_philosophers(t_data *data);
int					concurent_print(const char *message, t_data *data);
int					philo_print(int id, const char *message, t_data *data);
void				cleanup_data(t_data *data);
void				*philosophers(void *arg);
void				*monitoring_thread(void *arg);
int					get_current_time(void);
int					fragmented_sleep(int milliseconds, t_data *data);
int					ft_strlen(const char *str);
int					lock_fork(pthread_mutex_t *fork, t_philosopher *philo,
						t_data *data);
int					acquire_forks(t_philosopher *philo);
int					release_forks(t_philosopher *philo);
int					philo_eat(t_philosopher *philo);
int					start_monitoring_thread(t_data *data);

#endif
