/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_threads(t_sim *sim)
{
	int	i;

	sim->start_time = now_ms();
	i = 0;
	while (i < sim->cfg.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
		{
			fprintf(stderr, "Error: failed to create coder thread\n");
			stop_simulation(sim);
			return (0);
		}
		sim->coder_threads_started++;
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
	{
		fprintf(stderr, "Error: failed to create monitor thread\n");
		stop_simulation(sim);
		return (0);
	}
	sim->monitor_started = 1;
	return (1);
}

static void	join_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->coder_threads_started)
		pthread_join(sim->coders[i++].thread, NULL);
	if (sim->monitor_started)
		pthread_join(sim->monitor, NULL);
}

int	main(int argc, char **argv)
{
	t_sim	sim;
	int		status;

	memset(&sim, 0, sizeof(sim));
	if (!parse_args(argc, argv, &sim.cfg))
		return (1);
	if (sim.cfg.number_of_compiles_required == 0)
		return (0);
	status = 1;
	if (init_sim(&sim) && start_threads(&sim))
		status = 0;
	join_threads(&sim);
	cleanup_sim(&sim);
	return (status);
}
