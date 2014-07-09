// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
	if (pthread_mutex_init(&lock_mutex, NULL) != 0 ){               
		perror("Mutex Initialization Failed");
		exit(EXIT_FAILURE);
	}
	if( pthread_cond_init(&lock_cond, NULL) != 0 ){
		perror("Cond Initialization Failed");
		exit(EXIT_FAILURE);
	}
}


lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  nacquire++;
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  //printf("acquire request from clt %d\n", clt);
  
  pthread_mutex_lock(&lock_mutex);

  lock_map.insert(make_pair(lid,"free"));
  who_map.insert(make_pair(lid,0));
  while(lock_map[lid]=="locked")
	  pthread_cond_wait(&lock_cond, &lock_mutex);
  lock_map[lid] = "locked";
  who_map[lid] = clt;

  pthread_mutex_unlock(&lock_mutex);

  r = ++nacquire;
  lock_protocol::status ret = lock_protocol::OK;
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
//  printf("release request from clt %d\n", clt);
  
  pthread_mutex_lock(&lock_mutex);

  if(clt == who_map[lid])
  {
	lock_map[lid] = "free";
	who_map[lid] = 0;
	pthread_cond_signal(&lock_cond);
  }
  pthread_mutex_unlock(&lock_mutex);

  lock_protocol::status ret = lock_protocol::OK;
  return ret;
}
