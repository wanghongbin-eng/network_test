#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXLINES 1024

int client_run(int fd)
{
	char buf[MAXLINES];
	ssize_t len = 0;
	
	while(1)
	{
		len = recv(fd, buf, MAXLINES, 0);
		if(len == 0)
			break;
		else if(len == -1)
		{
			printf("recv error\n");
			exit(0);
		}

		send(fd, buf, len, 0);

		memset(buf, 0, MAXLINES);
	}
	close(fd);
	return 0;
}

void *pthread_run(void *arg)
{
	pthread_detach(pthread_self());
	int *fd = (int *)arg;
	client_run(*fd);
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

	if(listen(sock_fd, 100) < 0)
	{
		printf("listen error\n");
		exit(0);
	}

	return sock_fd;
}

int main(int argc, char **argv)
{
	int sock_fd = 0;
	int fd = 0;
	pthread_t tid;

	if(argc != 2)
	{
		printf("input port\n");
		exit(0);
	}

	sock_fd = tcp_server_listen(atoi(argv[1]));

	while(1)
	{
		struct sockaddr_in addr_c;
		int size = sizeof(struct sockaddr_in);
		memset(&addr_c, 0, size);

		fd = accept(sock_fd, (struct sockaddr *)&addr_c, &size);
		if(fd < 0)
		{
			printf("accept error\n");
			exit(0);
		}
	
		pthread_create(&tid, NULL, pthread_run, (void *)&fd);

	}

	return 0;
}
