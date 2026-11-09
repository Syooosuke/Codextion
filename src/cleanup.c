/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_sim_mutexes(t_sim *sim)
{
	if (sim->sim_mutexes_ready > 2)
		pthread_mutex_destroy(&sim->ticket_mutex);
	if (sim->sim_mutexes_ready > 1)
		pthread_mutex_destroy(&sim->stop_mutex);
	if (sim->sim_mutexes_ready > 0)
		pthread_mutex_destroy(&sim->log_mutex);
}

void	cleanup_sim(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->coder_mutexes_ready)
		pthread_mutex_destroy(&sim->coders[i++].mutex);
	i = 0;
	while (i < sim->dongle_mutexes_ready)
		pthread_mutex_destroy(&sim->dongles[i++].mutex);
	destroy_sim_mutexes(sim);
	free(sim->coders);
	free(sim->dongles);
}
