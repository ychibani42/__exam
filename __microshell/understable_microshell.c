#include "microshell.h"

int	__is_delim(char *str)
{
	if (!strcmp(str, ";"))
		return (1);
	return (0);
}

int	__is_pipe(char *str)
{
	if (!strcmp(str, "|"))
		return (1);
	return (0);
}

int	__is_builtin(char *str)
{
	if (!strcmp(str, "cd"))
		return (1);
	return (0);
}

void	__msh_putstr_fd(char *str, char *err)
{
	while (*str)
		write(2, str++, 1);
	if (err)
		while (*err)
			write (2, err++, 1);
	write (2, "\n", 1);
}

int	__cd(char *path, int index)
{
	if (index != 2)
		return (__msh_putstr_fd("error: cd: bad arguments", NULL), __SUCCESS);
	else if (chdir(path) != 0)
		return (__msh_putstr_fd("error: cd: cannot change directory to", path), __SUCCESS);
}

int	__exec(char **arg, int index, char **envp, int *prev_fd)
{
	arg[index] = NULL;
	dup2(*prev_fd, STDIN_FILENO);
	close(*prev_fd);
	execve(arg[0], arg, envp);
	return (__msh_putstr_fd("error: fatal\n", arg[0]), __FAILURE);
}

int	simple_fork(char **arg, int index, char **envp, int *prev_fd)
{
	int	pid;

	pid = fork();
	if (pid < 0)
		return (__FAILURE);
	if (!pid)
	{
		if (!__exec(arg, index, envp, prev_fd))
			return (__FAILURE);
	}
	else
	{
		close(*prev_fd);
		while (waitpid(-1, NULL, WUNTRACED) != __FAILURE)
			;
		*prev_fd = dup(STDIN_FILENO);
	}
}

int	complexe_fork(char **arg, int index, char **envp, int *prev_fd)
{
	int fds[2];
	int	pid;

	if (pipe(fds) < 0)
		return (__FAILURE);
	if ((pid = fork()) < 0)
		return (__FAILURE);
	if (!pid)
	{
		dup2(fds[1], STDOUT_FILENO);
		close(fds[0]);
		close(fds[1]);
		if (!__exec(arg, index, envp, prev_fd))
			return (__FAILURE);
	}
	else
	{
		close(fds[1]);
		close(*prev_fd);
		*prev_fd = fds[0];
	}
}

int	__microshell(char **av, char **envp)
{
	int		i;
	int		prev_fd;

	i = 0;
	prev_fd = dup(STDIN_FILENO);
	while (av[i] && av[i + 1])
	{
		av = &av[i + 1];
		i = 0;
		while (av[i] && !__is_delim(av[i]) && !__is_pipe(av[i]))
			i++;
		if (__is_builtin(av[0]))
		{
			if (__cd(av[1], i) == __FAILURE)
				return (__FAILURE);
		}
		else if (i != 0 && (!av[i] || __is_delim(av[i])))
		{
			if (simple_fork(av, i, envp, &prev_fd) == __FAILURE)
				return (__FAILURE);
		}
		else if (i != 0 && __is_pipe(av[i]))
		{
			if (complexe_fork(av, i, envp, &prev_fd) == __FAILURE)
				return (__FAILURE);
		}
	}
	close(prev_fd);
	return (__SUCCESS);
}

int	main(int ac, char **av, char **envp)
{
	if (ac < 2)
		return (__FAILURE);
	if (__microshell(av, envp) == __FAILURE);
		return (__FAILURE);
	return (__SUCCESS);
}
