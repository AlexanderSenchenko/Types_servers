#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

void init_server();
void sighandler(int);
void* handler(void*);

int sfd;

int main()
{
	int cfd;
	int ret;
	char buf[64];
	pthread_t tid;
	
	init_server();

	while (1) {
		cfd = accept(sfd, NULL, NULL);
		if (cfd == -1) {
			perror("accept");
			continue;
		}

		pthread_create(&tid, NULL, handler, (void*) &cfd);
	}

	return 0;
}

void* handler(void* ptr)
{
	int* ptr1 = (int*) ptr;
	int cfd = *ptr1;
	int ret;
	char buf[10];

	ret = read(cfd, buf, 10);
	if (ret == -1) {
		perror("Thread: read");
		pthread_exit(NULL);
	}

	buf[ret++] = '\0';
	printf("%s\n", buf);

	sleep(10);

	ret = write(cfd, "Hi", 2);
	if (ret == -1) {
		perror("Thread: write");
		pthread_exit(NULL);
	}

	close(cfd);

	return NULL;
}

void sighandler(int sig)
{
	shutdown(sfd, SHUT_RDWR);
	close(sfd);
	exit(EXIT_SUCCESS);
}

void init_server()
{
	int ret;
	int s_addr;
	struct sockaddr_in addr;

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
	addr.sin_addr.s_addr = (u_int32_t) s_addr;

	ret = bind(sfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("bind");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	ret = listen(sfd, 5);
	if (ret == -1) {
		perror("listen");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, sighandler);
}
