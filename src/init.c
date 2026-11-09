/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_sim_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (0);
	sim->sim_mutexes_ready++;
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (0);
	sim->sim_mutexes_ready++;
	if (pthread_mutex_init(&sim->ticket_mutex, NULL) != 0)
		return (0);
	sim->sim_mutexes_ready++;
	return (1);
}

static int	init_dongles(t_sim *sim)
{
	t_dongle	*dongle;
	int			i;

	i = 0;
	while (i < sim->cfg.number_of_coders)
	{
		dongle = &sim->dongles[i];
		dongle->in_use = 0;
		dongle->available_at = 0;
		dongle->queue.size = 0;
		if (pthread_mutex_init(&dongle->mutex, NULL) != 0)
			return (0);
		sim->dongle_mutexes_ready++;
		i++;
	}
	return (1);
}

static int	init_coders(t_sim *sim)
{
	t_coder	*coder;
	int		count;
	int		i;

	count = sim->cfg.number_of_coders;
	i = 0;
	while (i < count)
	{
		coder = &sim->coders[i];
		coder->id = i + 1;
		coder->left = &sim->dongles[coder->id - 1];
		coder->right = &sim->dongles[coder->id % count];
		coder->last_compile_start = 0;
		coder->compile_count = 0;
		coder->sim = sim;
		if (pthread_mutex_init(&coder->mutex, NULL) != 0)
			return (0);
		sim->coder_mutexes_ready++;
		i++;
	}
	return (1);
}

int	init_sim(t_sim *sim)
{
	sim->dongles = malloc(sizeof(t_dongle) * sim->cfg.number_of_coders);
	sim->coders = malloc(sizeof(t_coder) * sim->cfg.number_of_coders);
	if (sim->dongles == NULL || sim->coders == NULL)
	{
		fprintf(stderr, "Error: failed to allocate memory\n");
		return (0);
	}
	if (!init_sim_mutexes(sim) || !init_dongles(sim) || !init_coders(sim))
	{
		fprintf(stderr, "Error: failed to initialize mutex\n");
		return (0);
	}
	return (1);
}
