/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	take_ticket(t_sim *sim)
{
	long long	ticket;

	pthread_mutex_lock(&sim->ticket_mutex);
	sim->next_ticket++;
	ticket = sim->next_ticket;
	pthread_mutex_unlock(&sim->ticket_mutex);
	return (ticket);
}

int	has_priority(t_request a, t_request b, t_scheduler scheduler)
{
	if (scheduler == EDF && a.deadline != b.deadline)
		return (a.deadline < b.deadline);
	return (a.ticket < b.ticket);
}
