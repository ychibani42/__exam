#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

int sockfd;
int client_fd;
int	fd_max;
int	client = 0;
socklen_t	len;
fd_set	set_fd;
fd_set	set_fd_read;
fd_set	set_fd_write;
struct sockaddr_in	server_addr;
struct sockaddr_in	client_addr; 
int					_usr[65000];
char				*_msg[65000];
char				buffer[1025];
char				send_info[50];

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *s1, char *s2)
{
	char	*dst;
	int		len;

	len = 0;
	if (s1 != 0)
		len = strlen(s1);
	dst = malloc(sizeof(*dst) * (len + strlen(s2) + 1));
	if (dst == 0)
		return (0);
	dst[0] = 0;
	if (s1 != 0)
		strcat(dst, s1);
	free(s1);
	strcat(dst, s2);
	return (dst);
}

void	send_all(int from, char *str)
{
	for (int fd = 0; fd <= fd_max; fd++)
	{
		if (FD_ISSET(fd, &set_fd_write) && fd != from)
			send(fd, str, strlen(str), 0);
	}
}

void	fatal(void)
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit (1);
}

int main(int ac, char **av)
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit (1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		fatal(); 
	bzero(&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	server_addr.sin_port = htons(atoi(av[1])); 
	if ((bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) != 0) 
		fatal();
	if (listen(sockfd, 1024) != 0)
		fatal();
	FD_ZERO(&set_fd);
	FD_SET(sockfd, &set_fd);
	fd_max = sockfd;

	for (;;)
	{
		set_fd_read = set_fd_write = set_fd;
		if (select(fd_max + 1, &set_fd_read, &set_fd_write, 0, 0) < 0)
			fatal();
		for (int fd = 0; fd <= fd_max; fd++)
		{
			if (!FD_ISSET(fd, &set_fd_read))
				continue;
			if (fd == sockfd)
			{
				len = sizeof(client_addr);
				if ((client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) < 0)
					fatal();
				if (fd_max < client_fd)
					fd_max = client_fd;
				_usr[client_fd] = client;
				client++;
				_msg[client_fd] = NULL;
				sprintf(send_info, "server: client %d just arrived\n", _usr[client_fd]);
				send_all(client_fd, send_info);
				FD_SET(client_fd, &set_fd);
				break;
			}
			else
			{
				int	ret = recv(fd, buffer, 1024, 0);
				if (ret <= 0)
				{
					sprintf(send_info, "server: client %d just left\n", _usr[fd]);
					send_all(fd, send_info);
					close(fd);
					free(_msg[fd]);
					_msg[fd] = NULL;
					FD_CLR(fd, &set_fd);
					break;
				}
				buffer[ret] = '\0';
				_msg[fd] = str_join(_msg[fd], buffer);
				for (char *str = NULL; extract_message(&_msg[fd], &str);)
				{
					sprintf(send_info, "client %d: ", _usr[fd]);
					send_all(fd, send_info);
					send_all(fd, str);
					free(str);
					str = NULL;
				}
			}
		}
	}
	return (0);
}
