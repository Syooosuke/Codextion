/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
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

static int	has_priority(t_request a, t_request b, t_scheduler scheduler)
{
	if (scheduler == EDF && a.deadline != b.deadline)
		return (a.deadline < b.deadline);
	return (a.ticket < b.ticket);
}

static void	swap_requests(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_push(t_heap *heap, t_request request, t_scheduler scheduler)
{
	int	child;
	int	parent;

	child = heap->size;
	heap->items[child] = request;
	heap->size++;
	parent = (child - 1) / 2;
	while (child > 0
		&& has_priority(heap->items[child], heap->items[parent], scheduler))
	{
		swap_requests(&heap->items[child], &heap->items[parent]);
		child = parent;
		parent = (child - 1) / 2;
	}
}

void	heap_pop(t_heap *heap, t_scheduler scheduler)
{
	int	parent;
	int	child;

	heap->size--;
	heap->items[0] = heap->items[heap->size];
	parent = 0;
	child = 1;
	while (child < heap->size)
	{
		if (child + 1 < heap->size
			&& has_priority(heap->items[child + 1], heap->items[child],
				scheduler))
			child++;
		if (!has_priority(heap->items[child], heap->items[parent], scheduler))
			return ;
		swap_requests(&heap->items[child], &heap->items[parent]);
		parent = child;
		child = parent * 2 + 1;
	}
}
