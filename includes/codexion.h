/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

# define ARG_COUNT 8
# define POLL_INTERVAL_US 500
# define MONITOR_INTERVAL_US 1000
# define HEAP_CAPACITY 2

# define MSG_TAKEN "has taken a dongle"
# define MSG_COMPILING "is compiling"
# define MSG_DEBUGGING "is debugging"
# define MSG_REFACTORING "is refactoring"
# define MSG_BURNED_OUT "burned out"

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_config
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

typedef struct s_request
{
	long long	ticket;
	long long	deadline;
}	t_request;

typedef struct s_heap
{
	t_request	items[HEAP_CAPACITY];
	int			size;
}	t_heap;

typedef struct s_dongle
{
	int				index;
	pthread_mutex_t	mutex;
	int				in_use;
	long long		available_at;
	t_heap			queue;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_dongle		*left;
	t_dongle		*right;
	pthread_mutex_t	mutex;
	long long		last_compile_start;
	int				compile_count;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	t_config		cfg;
	long long		start_time;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_t		monitor;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	ticket_mutex;
	int				stop_flag;
	long long		next_ticket;
	int				dongle_mutexes_ready;
	int				coder_mutexes_ready;
	int				sim_mutexes_ready;
	int				coder_threads_started;
	int				monitor_started;
}	t_sim;

int			parse_args(int argc, char **argv, t_config *cfg);

int			init_sim(t_sim *sim);
void		cleanup_sim(t_sim *sim);

void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);

int			take_dongles(t_coder *coder);
void		release_dongles(t_coder *coder);

long long	take_ticket(t_sim *sim);
void		heap_push(t_heap *heap, t_request request, t_scheduler scheduler);
void		heap_pop(t_heap *heap, t_scheduler scheduler);

void		log_state(t_coder *coder, const char *message);
void		log_burnout(t_coder *coder);
void		stop_simulation(t_sim *sim);
int			is_stopped(t_sim *sim);

long long	now_ms(void);
long long	sim_time(t_sim *sim);
int			sleep_ms(t_sim *sim, long long duration);

#endif
