/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	run_single_coder(t_coder *coder)
{
	log_state(coder, MSG_TAKEN);
	while (!is_stopped(coder->sim))
		usleep(POLL_INTERVAL_US);
}

static void	start_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->mutex);
	coder->last_compile_start = sim_time(coder->sim);
	pthread_mutex_unlock(&coder->mutex);
	log_state(coder, MSG_COMPILING);
}

static void	finish_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->mutex);
}

static int	compile(t_coder *coder)
{
	int	finished;

	if (!take_dongles(coder))
		return (0);
	start_compile(coder);
	finished = sleep_ms(coder->sim, coder->sim->cfg.time_to_compile);
	if (finished)
	{
		finish_compile(coder);
		log_state(coder, MSG_DEBUGGING);
	}
	release_dongles(coder);
	return (finished);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = arg;
	sim = coder->sim;
	if (sim->cfg.number_of_coders == 1)
	{
		run_single_coder(coder);
		return (NULL);
	}
	while (!is_stopped(sim))
	{
		if (!compile(coder))
			return (NULL);
		if (!sleep_ms(sim, sim->cfg.time_to_debug))
			return (NULL);
		log_state(coder, MSG_REFACTORING);
		if (!sleep_ms(sim, sim->cfg.time_to_refactor))
			return (NULL);
	}
	return (NULL);
}
