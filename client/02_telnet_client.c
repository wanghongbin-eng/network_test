#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024

int tcp_socket_cli(in_addr_t ip_addr, int port)
{
	int sock_fd = 0;
	int ret = 0;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1)
		printf("socket error \n");

	struct sockaddr_in addr_c;
	memset(&addr_c, 0, sizeof(struct sockaddr_in));
	addr_c.sin_family = AF_INET;
	addr_c.sin_port = htons(port);
	addr_c.sin_addr.s_addr = ip_addr;
	
	ret = connect(sock_fd, (struct sockaddr *)&addr_c, sizeof(struct sockaddr));
	if(ret < 0)
		printf("connect error\n");
	return sock_fd;
}

int main(int argc, char **argv)
{
	int sock_fd = 0;
	int ret = 0;

	if(argc != 3)
	{
		printf("input ip_addr and port\n");
		return 0;
	}
	
	sock_fd = tcp_socket_cli(inet_addr(argv[1]), atoi(argv[2]));

	char recv_buf[BUF_SIZE] = {0}, send_buf[BUF_SIZE] = {0};
	int n = 0;
	size_t rt = 0;

	fd_set readmask;
	fd_set allmasks;
	FD_ZERO(&allmasks);
	FD_SET(0, &allmasks);
	FD_SET(sock_fd, &allmasks);

	for(;;)
	{
		readmask = allmasks;
		
		ret = select(sock_fd + 1, &readmask, NULL, NULL, NULL);

		if(ret <= 0)
		{
			printf("select error \n");
			return 0;
		}
		printf("select ok \n");
		if(FD_ISSET(sock_fd, &readmask))
		{
			n = read(sock_fd, recv_buf, BUF_SIZE);
			if(n < 0)
			{
				printf("read error\n");
			}
			else if(n == 0)
			{
				printf("server terminated \n");
				exit(0);
			}

			printf("recv %s\n", recv_buf);
		}

		if(FD_ISSET(STDIN_FILENO, &readmask))
		{
			if(fgets(send_buf, BUF_SIZE, stdin) != NULL)
			{
				rt = write(sock_fd, send_buf, strlen(send_buf));
				if(rt < 0)
					printf("write error\n");
			}
			printf("send %s byte:%zu\n", send_buf, rt);
			if(strcmp(send_buf, "exit\n") == 0)
				exit(0);
		}
	
	}

}

