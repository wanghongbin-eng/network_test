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

struct block_queue{
	int number;
	int *fd;
	int front;
	int rear;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

void block_queue_init(struct block_queue *blockqueue, int num)
{
	blockqueue->number = num;
	blockqueue->fd = calloc(num, sizeof(int));
	blockqueue->front = 0;
	blockqueue->rear = 0;
	pthread_mutex_init(&blockqueue->mutex, NULL);
	pthread_cond_init(&blockqueue->cond, NULL);
}

void block_queue_push(struct block_queue *blockqueue, int fd)
{
	pthread_mutex_lock(&blockqueue->mutex);

	blockqueue->fd[blockqueue->rear] = fd;

	if(++blockqueue->rear == blockqueue->number)
		blockqueue->rear = 0;

	printf("push fd %d\n", fd);

	pthread_cond_signal(&blockqueue->cond);
	pthread_mutex_unlock(&blockqueue->mutex);
}

int block_queue_pop(struct block_queue *blockqueue)
{
	pthread_mutex_lock(&blockqueue->mutex);
	while(blockqueue->front == blockqueue->rear)
		pthread_cond_wait(&blockqueue->cond, &blockqueue->mutex);

	int fd = blockqueue->fd[blockqueue->front];

	if(++blockqueue->front == blockqueue->number)
		blockqueue->front = 0;

	printf("pop fd %d\n", fd);

	pthread_mutex_unlock(&blockqueue->mutex);
	
	return fd;
}

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

	printf("thread id %lu\n", pthread_self());
	struct block_queue *blockqueue = (struct block_queue *)arg;

	int fd = block_queue_pop(blockqueue);
	client_run(fd);
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
	
	if(argc != 3)
	{
		printf("input port and block_queue_size\n");
		exit(0);
	}

	sock_fd = tcp_server_listen(atoi(argv[1]));

	int block_queue_size = atoi(argv[2]);
	struct block_queue blockqueue;
	block_queue_init(&blockqueue, block_queue_size);

	pthread_t tid[block_queue_size];

	int i;
	int ret = 0;
	for(i = 0; i < block_queue_size; i++)
	{
		ret = pthread_create(&(tid[i]), NULL, &pthread_run, (void *)&blockqueue);
	}

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
	
		block_queue_push(&blockqueue, fd);

	}

	return 0;
}
