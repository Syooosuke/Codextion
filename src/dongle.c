/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_ready(t_dongle *dongle, long long ticket, long long now)
{
	return (!dongle->in_use
		&& now >= dongle->available_at
		&& dongle->queue.items[0].ticket == ticket);
}

static int	try_take(t_coder *coder, t_dongle *dongle, long long ticket)
{
	int	taken;

	pthread_mutex_lock(&dongle->mutex);
	taken = is_ready(dongle, ticket, sim_time(coder->sim));
	if (taken)
	{
		dongle->in_use = 1;
		heap_pop(&dongle->queue, coder->sim->cfg.scheduler);
	}
	pthread_mutex_unlock(&dongle->mutex);
	return (taken);
}

static int	wait_for_dongle(t_coder *coder, t_dongle *dongle, t_request request)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->queue, request, coder->sim->cfg.scheduler);
	pthread_mutex_unlock(&dongle->mutex);
	while (!try_take(coder, dongle, request.ticket))
	{
		if (is_stopped(coder->sim))
			return (0);
		usleep(POLL_INTERVAL_US);
	}
	log_state(coder, MSG_TAKEN);
	return (1);
}

int	take_dongles(t_coder *coder)
{
	t_request	request;
	t_dongle	*first;
	t_dongle	*second;

	if (coder->id % 2 == 1)
	{
		first = coder->right;
		second = coder->left;
	}
	else
	{
		first = coder->left;
		second = coder->right;
	}
	request.ticket = take_ticket(coder->sim);
	pthread_mutex_lock(&coder->mutex);
	request.deadline = coder->last_compile_start
		+ coder->sim->cfg.time_to_burnout;
	pthread_mutex_unlock(&coder->mutex);
	return (wait_for_dongle(coder, first, request)
		&& wait_for_dongle(coder, second, request));
}

void	release_dongles(t_coder *coder)
{
	long long	available_at;

	available_at = sim_time(coder->sim) + coder->sim->cfg.dongle_cooldown;
	pthread_mutex_lock(&coder->left->mutex);
	coder->left->in_use = 0;
	coder->left->available_at = available_at;
	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_lock(&coder->right->mutex);
	coder->right->in_use = 0;
	coder->right->available_at = available_at;
	pthread_mutex_unlock(&coder->right->mutex);
}
