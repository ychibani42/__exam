#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

int id = 0; int max; 
fd_set curr_fd, write_fd, read_fd;

typedef struct s_client
{
	int fd;
	int id;

} t_client;

t_client clients[1024];
char buffer[1000000], msg[1000000];
int sockfd;




void add_client(void)
{
	int i = 0;
	struct sockaddr_in cli; 

	while (clients[i].fd > 0 && i < 1024)
		i++;
	unsigned int len = sizeof(cli);
	clients[i].fd = accept(sockfd, (struct sockaddr *)&cli, &len);
	if (clients[i].fd < 0)
		fatal ();
	clients[i].id = id++;
	if (clients[i].fd > max)
		max = clients[i].fd;
	FD_SET(clients[i].fd, &curr_fd);
}


int main(int ac, char **av)
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}
	int  connfd, len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockfd, 10) != 0) {
		printf("cannot listen\n"); 
		exit(0); 
	}

	FD_ZERO(&curr_fd);
	FD_SET(sockfd, &curr_fd);
	max = sockfd;

	while (1)
	{
		write_fd = read_fd = curr_fd;	
		if (select(max + 1, &read_fd, &write_fd, NULL, NULL) < 0)
			fatal();
		if (FD_ISSET(sockfd, &read_fd))
		{
			add_client();
		}
		for (int i = 0; i < 1024; i++)
		{
			if (FD_ISSET(clients[i].fd, &read_fd));
			{
				int ret = 1;

				while (ret)
				{
					ret = recv(clients[i].fd, buffer + strlen(buffer), 1, 0);
					if (buffer[strlen(buffer) - 1] == '\n')
						break ; 
				}
				if (ret == 0)
				{
					// print(il s;est deco)
					FD_CLR(clients[i].fd, &curr_fd);
					close(clients[i].fd);
					clients[i].id = -1;
					clients[i].fd = -1;
				}
				else
				{
					broadcast();
				}
			}
		}


		/* code */
	}	
	if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n")
}