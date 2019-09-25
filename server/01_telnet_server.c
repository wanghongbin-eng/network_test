#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
	int sock_fd = 0;

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error\n");
	}

	struct sockaddr_in addr_s;
	memset(&addr_s, 0, sizeof(struct sockaddr_in));
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(atoi(argv[1]));
	addr_s.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock_fd, (struct sockaddr *)&addr_s, sizeof(struct sockaddr)) < 0)
	{
		printf("bind error \n");
		exit(1);
	}

	printf("port %d\n", atoi(argv[1]));

	if(listen(sock_fd, 10) < 0)
	{
		printf("listen error \n");
		exit(1);
	}

	char buffer[64] = {0};
	char temp[64] = {0};
	struct sockaddr_in addr_c;
	memset(&addr_c, 0, sizeof(struct sockaddr_in));
	
	int conn = 0;
	int size = sizeof(struct sockaddr);
	if((conn = accept(sock_fd, (struct sockaddr *)&addr_c, &size)) < 0)
	{
		printf("accept error\n");
		exit(1);
	}

	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		int len = recv(conn, buffer, sizeof(buffer), 0);
		if(strcmp(buffer, "exit\n") == 0 || len < 0)
			break;
		if(strcmp(buffer, "ls\n") == 0)
			sprintf(temp,"%s > a.a\n", "ls");
		system(temp);
		printf("temp %s\n", temp);
		printf("recv %s\n", buffer);
		send(conn, buffer, len, 0);
		printf("send %s\n", buffer);
	}
	close(conn);
	close(sock_fd);
	return 0;
}
