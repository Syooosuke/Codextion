/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	set_stop_flag(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop_flag = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
}

int	is_stopped(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop_flag;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void	stop_simulation(t_sim *sim)
{
	pthread_mutex_lock(&sim->log_mutex);
	set_stop_flag(sim);
	pthread_mutex_unlock(&sim->log_mutex);
}

void	log_state(t_coder *coder, const char *message)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	if (!is_stopped(sim))
		printf("%lld %d %s\n", sim_time(sim), coder->id, message);
	pthread_mutex_unlock(&sim->log_mutex);
}

void	log_burnout(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->log_mutex);
	set_stop_flag(sim);
	printf("%lld %d %s\n", sim_time(sim), coder->id, MSG_BURNED_OUT);
	pthread_mutex_unlock(&sim->log_mutex);
}
