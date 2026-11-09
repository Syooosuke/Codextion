/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	now_ms(void)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return (now.tv_sec * 1000LL + now.tv_usec / 1000);
}

long long	sim_time(t_sim *sim)
{
	return (now_ms() - sim->start_time);
}

int	sleep_ms(t_sim *sim, long long duration)
{
	long long	end;

	end = sim_time(sim) + duration;
	while (sim_time(sim) < end)
	{
		if (is_stopped(sim))
			return (0);
		usleep(POLL_INTERVAL_US);
	}
	return (!is_stopped(sim));
}
