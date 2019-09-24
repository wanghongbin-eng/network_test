#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int sock_fd = 0;
	
	if((sock_fd = socket(AF_INET,SOCK_STREAM, 0)) == -1)
	{
		printf("socket error \n");
	}

	struct sockaddr_in addr_c;
	memset(&addr_c, 0, sizeof(struct sockaddr_in));
	addr_c.sin_family = AF_INET;
	addr_c.sin_port = htons(atoi(argv[2]));
	addr_c.sin_addr.s_addr = inet_addr(argv[1]);
	printf("addr %s, port %d\n", argv[1], atoi(argv[2]));

	if(connect(sock_fd, (struct sockaddr *)&addr_c, sizeof(struct sockaddr)) < 0)
	{
		printf("connect error \n");
	}
	printf("connect ok \n");

	char sendbuf[64] = {0};
	char recvbuf[64] = {0};


	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		printf("send %s\n", sendbuf);
	
		send(sock_fd, &sendbuf, strlen(sendbuf), 0);

		recv(sock_fd, recvbuf, sizeof(recvbuf), 0);
		printf("recv %s\n", recvbuf);
	}
	close(sock_fd);
	return 0;
}
