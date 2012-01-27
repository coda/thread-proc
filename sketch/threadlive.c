#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

static void * routine(void * arg)
{
	sleep(10);
	printf("thread done\n");
	return NULL;
}

int main(int argc, char * argv[])
{
	pthread_t t;
	pthread_create(&t, NULL, routine, NULL);
	sleep(5);
	printf("process done\n");
	pthread_exit(NULL);
	return 0;
}
