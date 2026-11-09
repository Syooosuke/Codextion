/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_coder *coder)
{
	t_sim	*sim;
	int		burned_out;

	sim = coder->sim;
	pthread_mutex_lock(&coder->mutex);
	burned_out = sim_time(sim) - coder->last_compile_start
		>= sim->cfg.time_to_burnout;
	if (burned_out)
		log_burnout(coder);
	pthread_mutex_unlock(&coder->mutex);
	return (burned_out);
}

static int	has_finished(t_coder *coder)
{
	int	finished;

	pthread_mutex_lock(&coder->mutex);
	finished = coder->compile_count
		>= coder->sim->cfg.number_of_compiles_required;
	pthread_mutex_unlock(&coder->mutex);
	return (finished);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		finished_count;
	int		i;

	sim = arg;
	while (!is_stopped(sim))
	{
		finished_count = 0;
		i = 0;
		while (i < sim->cfg.number_of_coders)
		{
			if (check_burnout(&sim->coders[i]))
				return (NULL);
			finished_count += has_finished(&sim->coders[i]);
			i++;
		}
		if (finished_count == sim->cfg.number_of_coders)
		{
			stop_simulation(sim);
			return (NULL);
		}
		usleep(MONITOR_INTERVAL_US);
	}
	return (NULL);
}
