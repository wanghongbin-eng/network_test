#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

struct addr_s
{
	in_addr_t addr;
	int port;
	char buf[64];
};

void *thread_run(void *arg)
{
	pthread_detach(pthread_self());
	
	struct addr_s *addr = (struct addr_s *)arg;
	
	int sock_fd = 0;

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socker error\n");
		exit(0);
	}

	struct sockaddr_in addr_c;
	memset(&addr_c, 0, sizeof(struct sockaddr_in));
	addr_c.sin_family = AF_INET;
	addr_c.sin_port = addr->port;
	addr_c.sin_addr.s_addr = addr->addr;

	if(connect(sock_fd, (struct sockaddr *)&addr_c, sizeof(struct sockaddr)) < 0)
	{
		printf("connect error\n");
		exit(0);
	}
	printf("thread %lu\n", pthread_self());
	char recvbuf[16];
	while(1)
	{

		send(sock_fd, &(addr->buf), strlen(addr->buf), 0);

		sleep(1);
		recv(sock_fd, recvbuf, sizeof(recvbuf), 0);
		sleep(5);
		printf("thread %lu: send %s recv %s\n", pthread_self(), addr->buf, recvbuf);
		
	}
	close(sock_fd);
}

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		printf("input addr port  thread_num\n");
		exit(0);
	}
	
	struct addr_s addr;
	pthread_t tid[10];
	int i;

	memset(&addr, 0, sizeof(struct addr_s));
	addr.port = htons(atoi(argv[2]));
	addr.addr = inet_addr(argv[1]);

	printf("addr %d, port %d\n", addr.addr, ntohs(addr.port));

	for(i = 0; i < atoi(argv[3]); i++)
	{
		addr.buf[0] = '1';
		addr.buf[1] = '\0';

		pthread_create(&tid[i], NULL, thread_run, (void *)&addr);
	}


	for(i = 0; i < atoi(argv[3]); i++)
		pthread_join(tid[i], NULL);

	return 0;
}
