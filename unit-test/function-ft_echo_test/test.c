#include <stdio.h>
#include <libc.h>
#include "shell.h"

int	main(int ac, char **av)
{
	return (ft_echo(&av[1]));
}
