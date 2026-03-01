# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: zaddi <zaddi@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/03/01 17:00:00 by zaddi             #+#    #+#              #
#    Updated: 2026/03/01 16:52:52 by zaddi            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
INCLUDES = -I./includes
LDFLAGS =
RM = rm -f

SRCS = main.c \
       ft_init.c \
       ft_monitoring.c \
       ft_philosophers.c \
       ft_philo_routine.c \
       ft_philo_forks.c \
       ft_utils_philo.c \
       ft_utils.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
