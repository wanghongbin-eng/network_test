#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>


#define MAXEVENTS 128
#define MAXLINE 1024

int set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

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
	if(listen(sock_fd, 10) < 0)
	{
		printf("listen error\n");
		exit(0);
	}
	return sock_fd;
}

int main(int argc, char **argv)
{
	int sock_fd, connect_fd;
	int n, i;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	sock_fd = tcp_server_listen(atoi(argv[1]));
	printf("sock_fd %d\n", sock_fd);
	efd = epoll_create1(0);
	if(efd == -1)
	{
		printf("epoll_create error\n");
		exit(0);
	}

	memset(&event, 0, sizeof(event));
	event.data.fd = sock_fd;
	event.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &event) == -1)
	{
		printf("epoll_ctl error\n");
		exit(0);
	}

	events = calloc(MAXEVENTS, sizeof(event));

	while(1)
	{
		n = epoll_wait(efd, events, MAXEVENTS, 2000);
		printf("epoll_wait wakeup %d\n", n);
		for(i = 0; i < n; i++)
		{
			if((events[i].events & EPOLLERR) ||
			   (events[i].events & EPOLLHUP) ||
			   (!(events[i].events & EPOLLIN)))
			{
				printf("fd close\n");
				close(events[i].data.fd);
				continue;
			}
			else if(sock_fd == events[i].data.fd)
			{
				struct sockaddr_storage ss;
				socklen_t len = sizeof(ss);
				connect_fd = accept(sock_fd, (struct sockaddr *)&ss, &len);
				if(connect_fd < 0)
				{
					printf("accept error\n");
					exit(0);
				}
				else
				{
					//set_nonblocking(connect_fd);
					event.data.fd = connect_fd;
					event.events = EPOLLIN | EPOLLET;
					if(epoll_ctl(efd, EPOLL_CTL_ADD, connect_fd, &event) == -1)
					{
						printf("epoll_ctl error\n");
						exit(0);
					}
				}
				continue;
			}
			else
			{
				int fd = events[i].data.fd;
				printf("get event on socket fd = %d\n", fd);
				char buf[MAXLINE];
				if((n = read(fd, buf,MAXLINE)) < 0)
				{
					printf("read error \n");
					close(sock_fd);
					break;
				}
				else if(n == 0)
				{
					printf("read error return = %d\n", n);
					close(sock_fd);
					break;
				}
				else
				{
					if(write(fd, buf, n) < 0)
					{
						printf("write error\n");
						exit(0);
					}
					break;
				}
			}
		}
	}
	free(events);
	close(sock_fd);
}




























