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

static void	swap_requests(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	sift_up(t_heap *heap, int child, t_scheduler scheduler)
{
	int	parent;

	parent = (child - 1) / 2;
	while (child > 0
		&& has_priority(heap->items[child], heap->items[parent], scheduler))
	{
		swap_requests(&heap->items[child], &heap->items[parent]);
		child = parent;
		parent = (child - 1) / 2;
	}
}

static void	sift_down(t_heap *heap, int parent, t_scheduler scheduler)
{
	int	child;

	child = parent * 2 + 1;
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

void	heap_push(t_heap *heap, t_request request, t_scheduler scheduler)
{
	heap->items[heap->size] = request;
	heap->size++;
	sift_up(heap, heap->size - 1, scheduler);
}

void	heap_remove(t_heap *heap, long long ticket, t_scheduler scheduler)
{
	int	i;

	i = 0;
	while (heap->items[i].ticket != ticket)
		i++;
	heap->size--;
	if (i == heap->size)
		return ;
	heap->items[i] = heap->items[heap->size];
	sift_down(heap, i, scheduler);
	sift_up(heap, i, scheduler);
}
