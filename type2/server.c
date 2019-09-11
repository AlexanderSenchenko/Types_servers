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
void init_pthread(int num);

int sfd;
int cfd[3];
int status[3];
pthread_t tid[3];
pthread_mutex_t synch_init;
pthread_mutex_t mutex[3];

int main()
{
	int new_cfd;
	int ret;
	int i, j;

	init_server();

	init_pthread(3);

	for (i = 0; i < 3; ++i)
		status[i] = 0;

	while (1) {
		new_cfd = accept(sfd, NULL, NULL);
		if (new_cfd == -1) {
			perror("accept");
			continue;
		}

		while (1) {
			for (i = 0; i < 3; ++i) {
				// if (status[i] == 0)
					// status[i] = 1;
					// cfd[i] = new_cfd;
					// break;
				
				ret = pthread_mutex_trylock(&mutex[i]);
				if (ret == 0)
					break;
			}

			if (i != 4)
				break;
		}

		// for (j = 0; j < 3; ++j) {
		// 	printf("%d ", status[j]);
		// }
		// printf("\n");

		// for (j = 0; j < 3; ++j) {
		// 	printf("%d ", cfd[j]);
		// }
		// printf("\n");

		// printf("id_tid=%d\n", i);

		// pthread_mutex_unlock(&mutex[i]);
	}

	return 0;
}

void init_pthread(int num)
{
	int i;
	pthread_mutex_init(&synch_init, NULL);

	for (i = 0; i < num; ++i) {
		pthread_mutex_init(&mutex[i], NULL);
		pthread_mutex_lock(&mutex[i]);

		pthread_mutex_lock(&synch_init);
		pthread_create(&tid[i], NULL, handler, (void*) &i);
	}
}

void* handler(void* ptr)
{
	int i = *((int*) ptr);
	int ret;
	char buf[10];

	pthread_mutex_unlock(&synch_init);

	while (1) {
		pthread_mutex_lock(&mutex[i]);

		ret = read(cfd[i], buf, 10);
		buf[ret++] = '\0';
		printf("id_tid=%d\nmsg=%s\n", i, buf);

		sleep(10);

		write(cfd[i], "Hi", 2);

		status[i] = 0;
		close(cfd[i]);
		memset(buf, 0, ret);
	}
}

void sighandler(int sig)
{
	int i;
	for (i = 0; i < 3; ++i) {
		pthread_cancel(tid[i]);
	}

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
