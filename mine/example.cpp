/* example.cpp*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
using namespace std;
void *thread(void *ptr)
{
	int i;
	for (i = 0 ; i < 3; i++)
	{
	sleep(5);
	printf("\nThis is thread %d",i);
	}
}

int main(void)
{
	pthread_t id;
	int i,ret;
	ret = pthread_create(&id,NULL,thread,NULL);

	if(ret!=0){
	
		cout<<"\nCreate thread error!"<<endl;
		exit(1);
	}
	for(i=0;i<3;i++){
		printf("\nThis is main process %d",i);
		sleep(1);
	}
	pthread_join(id,NULL);
	printf("\nHere!\n");
	return (0);
}
