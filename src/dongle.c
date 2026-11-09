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

static void	join_queue(t_coder *coder, t_dongle *dongle, t_request request)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->queue, request, coder->sim->cfg.scheduler);
	pthread_mutex_unlock(&dongle->mutex);
}

static int	can_take(t_coder *coder, t_dongle *dongle, long long ticket,
	int is_second)
{
	t_config	*cfg;
	t_request	head;
	long long	now;

	cfg = &coder->sim->cfg;
	head = dongle->queue.items[0];
	now = sim_time(coder->sim);
	if (dongle->in_use || now < dongle->available_at)
		return (0);
	if (head.ticket == ticket)
		return (1);
	return (is_second
		&& head.deadline > now + cfg->time_to_compile + cfg->dongle_cooldown);
}

static int	wait_for_dongle(t_coder *coder, t_dongle *dongle, long long ticket,
	int is_second)
{
	int	taken;

	taken = 0;
	while (!taken)
	{
		pthread_mutex_lock(&dongle->mutex);
		taken = can_take(coder, dongle, ticket, is_second);
		if (taken)
		{
			dongle->in_use = 1;
			heap_remove(&dongle->queue, ticket, coder->sim->cfg.scheduler);
		}
		pthread_mutex_unlock(&dongle->mutex);
		if (!taken && is_stopped(coder->sim))
			return (0);
		if (!taken)
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

	first = coder->left;
	second = coder->right;
	if (coder->id % 2 == 1)
	{
		first = coder->right;
		second = coder->left;
	}
	request.ticket = take_ticket(coder->sim);
	pthread_mutex_lock(&coder->mutex);
	request.deadline = coder->last_compile_start
		+ coder->sim->cfg.time_to_burnout;
	pthread_mutex_unlock(&coder->mutex);
	join_queue(coder, first, request);
	join_queue(coder, second, request);
	return (wait_for_dongle(coder, first, request.ticket, 0)
		&& wait_for_dongle(coder, second, request.ticket, 1));
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
