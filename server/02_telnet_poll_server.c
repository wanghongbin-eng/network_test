#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>

#define INIT_SIZE 128
#define MAXLINE 1024

int tcp_server_listen(int port)
{
	int sock_fd = 0;

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error\n");
		exit(0);
	}

	struct sockaddr_in addr_s;
	memset(&addr_s, 0, sizeof(struct sockaddr_in));
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(port);
	addr_s.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock_fd, (struct sockaddr *)&addr_s, sizeof(struct sockaddr)) < 0)
	{
		printf("bind error\n");
		exit(0);
	}

	if(listen(sock_fd, INIT_SIZE) < 0)
	{
		printf("listen error\n");
		exit(0);
	}

	return sock_fd;
}

int main(int argc, char **argv)
{
	int sock_fd, connect_fd;
	int ready_num;
	ssize_t n;
	char buf[MAXLINE];
	struct sockaddr_in addr_c;

	if(argc != 2)
	{
		printf("input port\n");
		exit(0);
	}

	sock_fd = tcp_server_listen(atoi(argv[1]));
	
	struct pollfd event_set[INIT_SIZE];
	event_set[0].fd = sock_fd;
	event_set[0].events = POLLRDNORM;

	int i = 1;
	for(i; i < INIT_SIZE; i++)
		event_set[i].fd = -1;

	for(;;)
	{
		if((ready_num = poll(event_set, INIT_SIZE, 2000)) < 0)
		{
			printf("poll error\n");
			exit(0);
		}
		printf("poll ok ready_num %d\n", ready_num);
		if(event_set[0].revents & POLLRDNORM)
		{
			socklen_t client_len = sizeof(addr_c);
			connect_fd = accept(sock_fd, (struct sockaddr *)&addr_c, &client_len);
			for(i = 1; i < INIT_SIZE; i++)
			{
				if(event_set[i].fd < 0)
				{
					event_set[i].fd = connect_fd;
					event_set[i].events = POLLRDNORM;
					break;
				}
			}
			
			if(i == INIT_SIZE)
			{
				printf("can not hold so many clients\n");
				exit(0);
			}

			if(--ready_num <= 0)
				continue;
		}

		for(i = 1; i < INIT_SIZE; i++)
		{
			int socket_fd;
			if((socket_fd = event_set[i].fd) < 0)
				continue;

			if(event_set[i].revents & (POLLRDNORM | POLLERR))
			{
				if((n = read(socket_fd, buf, MAXLINE)) > 0)
				{
					if(write(socket_fd, buf, n) < 0)
					{
						printf("write error\n");
						exit(0);
					}
				}
				else if(n == 0)
				{
					close(socket_fd);
					event_set[i].fd = -1;
				}
				else
				{
					printf("read error\n");
					exit(0);
				}
			}
			
			if(--ready_num <= 0)
				break;
		}

	}

}
