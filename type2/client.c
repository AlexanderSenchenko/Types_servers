#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int sfd;
	int ret;
	struct sockaddr_in addr;
	int s_addr;
	char buf[10];

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	ret = inet_pton(AF_INET, "127.0.0.1", &s_addr);
	if (ret == 0) {
		perror("inet_pton(0)");
		close(sfd);
		exit(EXIT_FAILURE);
	} else if (ret == -1) {
		perror("inet_pton(-1)");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	addr.sin_addr.s_addr = s_addr;

	ret = connect(sfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("connect");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	write(sfd, "Hello", 5);
	
	sleep(10);

	ret = read(sfd, buf, 10);
	buf[ret++] = '\0';
	printf("%s\n", buf);

	shutdown(sfd, SHUT_RDWR);
	close(sfd);

	return 0;
}
