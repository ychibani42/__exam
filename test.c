#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    fd_set readfds, writefds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

	FD_ZERO(&writefds);
    FD_SET(STDOUT_FILENO, &readfds);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int ret = select(STDIN_FILENO + 1 + 1, &readfds, &writefds, NULL, &timeout);
    if (ret == -1) 
	{
        perror("select");
        exit(EXIT_FAILURE);
    } else if (ret == 0) 
	{
        printf("No data within five seconds.\n");
    } 
	else 
	{
        if (FD_ISSET(STDOUT_FILENO, &readfds))
		{
			char buffer[1024];
			recv(STDOUT_FILENO, buffer, 1024, 0);
            printf("Data is available now.\n");
			printf("%s", buffer);
        }
    }

    return 0;
}
