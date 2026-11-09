/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syokota <syokota@student.42tokyo.jp>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 11:00:00 by syokota           #+#    #+#             */
/*   Updated: 2026/11/09 11:00:00 by syokota          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_digits_only(const char *str)
{
	int	i;

	if (str[0] == '\0')
		return (0);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	check_range(const char *name, const char *str, long value, int min)
{
	if (value > INT_MAX)
	{
		fprintf(stderr, "Error: %s: '%s' is too large (max %d)\n",
			name, str, INT_MAX);
		return (0);
	}
	if (value < min)
	{
		fprintf(stderr, "Error: %s: '%s' must be at least %d\n",
			name, str, min);
		return (0);
	}
	return (1);
}

static int	parse_number(const char *name, const char *str, int min, int *out)
{
	long	value;
	int		i;

	if (!is_digits_only(str))
	{
		fprintf(stderr, "Error: %s: '%s' is not a non-negative integer\n",
			name, str);
		return (0);
	}
	value = 0;
	i = 0;
	while (str[i] && value <= INT_MAX)
		value = value * 10 + (str[i++] - '0');
	if (!check_range(name, str, value, min))
		return (0);
	*out = (int)value;
	return (1);
}

static int	parse_scheduler(const char *str, t_scheduler *out)
{
	if (strcmp(str, "fifo") == 0)
		*out = FIFO;
	else if (strcmp(str, "edf") == 0)
		*out = EDF;
	else
	{
		fprintf(stderr, "Error: scheduler: '%s' must be 'fifo' or 'edf'\n",
			str);
		return (0);
	}
	return (1);
}

int	parse_args(int argc, char **argv, t_config *cfg)
{
	if (argc - 1 != ARG_COUNT)
	{
		fprintf(stderr, "Error: expected %d arguments, got %d\n",
			ARG_COUNT, argc - 1);
		return (0);
	}
	return (parse_number("number_of_coders", argv[1], 1,
			&cfg->number_of_coders)
		&& parse_number("time_to_burnout", argv[2], 0, &cfg->time_to_burnout)
		&& parse_number("time_to_compile", argv[3], 0, &cfg->time_to_compile)
		&& parse_number("time_to_debug", argv[4], 0, &cfg->time_to_debug)
		&& parse_number("time_to_refactor", argv[5], 0,
			&cfg->time_to_refactor)
		&& parse_number("number_of_compiles_required", argv[6], 0,
			&cfg->number_of_compiles_required)
		&& parse_number("dongle_cooldown", argv[7], 0, &cfg->dongle_cooldown)
		&& parse_scheduler(argv[8], &cfg->scheduler));
}
