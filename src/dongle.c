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

static void	join_queues(t_coder *coder, t_request request)
{
	t_scheduler	scheduler;

	scheduler = coder->sim->cfg.scheduler;
	lock_dongles(coder);
	heap_push(&coder->left->queue, request, scheduler);
	heap_push(&coder->right->queue, request, scheduler);
	unlock_dongles(coder);
}

static int	is_ready(t_dongle *dongle, long long ticket, long long now)
{
	return (!dongle->in_use
		&& now >= dongle->available_at
		&& dongle->queue.items[0].ticket == ticket);
}

static int	try_take(t_coder *coder, long long ticket)
{
	t_scheduler	scheduler;
	long long	now;
	int			taken;

	scheduler = coder->sim->cfg.scheduler;
	lock_dongles(coder);
	now = sim_time(coder->sim);
	taken = is_ready(coder->left, ticket, now)
		&& is_ready(coder->right, ticket, now);
	if (taken)
	{
		coder->left->in_use = 1;
		coder->right->in_use = 1;
		heap_pop(&coder->left->queue, scheduler);
		heap_pop(&coder->right->queue, scheduler);
	}
	unlock_dongles(coder);
	return (taken);
}

int	take_dongles(t_coder *coder)
{
	t_request	request;

	request.ticket = take_ticket(coder->sim);
	pthread_mutex_lock(&coder->mutex);
	request.deadline = coder->last_compile_start
		+ coder->sim->cfg.time_to_burnout;
	pthread_mutex_unlock(&coder->mutex);
	join_queues(coder, request);
	while (!try_take(coder, request.ticket))
	{
		if (is_stopped(coder->sim))
			return (0);
		usleep(POLL_INTERVAL_US);
	}
	return (1);
}

void	release_dongles(t_coder *coder)
{
	long long	available_at;

	lock_dongles(coder);
	available_at = sim_time(coder->sim) + coder->sim->cfg.dongle_cooldown;
	coder->left->in_use = 0;
	coder->left->available_at = available_at;
	coder->right->in_use = 0;
	coder->right->available_at = available_at;
	unlock_dongles(coder);
}
