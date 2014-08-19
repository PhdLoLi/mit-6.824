// RPC stubs for clients to talk to lock_server, and cache the locks
// see lock_client.cache.h for protocol details.

#include "lock_client_cache.h"
#include "rpc.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include "tprintf.h"
#include <map>

lock_client_cache::lock_client_cache(std::string xdst, 
				     class lock_release_user *_lu)
  : lock_client(xdst), lu(_lu)
{
  rpcs *rlsrpc = new rpcs(0);
  rlsrpc->reg(rlock_protocol::revoke, this, &lock_client_cache::revoke_handler);
  rlsrpc->reg(rlock_protocol::retry, this, &lock_client_cache::retry_handler);

  const char *hname;
  hname = "127.0.0.1";
  std::ostringstream host;
  host << hname << ":" << rlsrpc->port();
  id = host.str();

  sockaddr_in dstsock;
  make_sockaddr(xdst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() < 0) {
    printf("lock_client: call bind\n");
  }
  printf("lock client port :%s init OK\n",xdst.c_str());
	std::cout<<"id: "<<id<<std::endl;
}

lock_protocol::status
lock_client_cache::acquire(lock_protocol::lockid_t lid)
{
	int r;
  int ret = lock_protocol::OK;
	//lock here!
	//pthread_mutex_lock(&lock_mutex);
	pthread_mutex_t mutex_temp;
	pthread_cond_t cond_temp;
	if (pthread_mutex_init(&mutex_temp, NULL) != 0 ){               
		perror("Mutex Initialization Failed");
		exit(EXIT_FAILURE);
	}
	if( pthread_cond_init(&cond_temp, NULL) != 0 ){
		perror("Cond Initialization Failed");
		exit(EXIT_FAILURE);
	}

	lock_mutex_map.insert(std::make_pair(lid, mutex_temp));
	lock_cond_map.insert(std::make_pair(lid, cond_temp));
	//owner_map.insert(make_pair(lid, pthread_self()));

	pthread_mutex_lock(&lock_mutex_map[lid]);
	lock_map.insert(std::make_pair(lid, NONE));	
	
//RELEASING LOCKED ACQUIRING waiting 
	while(lock_map[lid] == RELEASING || lock_map[lid] == LOCKED || lock_map[lid] == ACQUIRING)
	{
		printf("acquire-- lid:%llu\tstatus/lock_map[lid]: %d  I'm waiting\n",lid,lock_map[lid]);
		pthread_cond_wait(&lock_cond_map[lid], &lock_mutex_map[lid]);
	}

	if(lock_map[lid] == NONE)
	{
		lock_map[lid] = ACQUIRING;

		pthread_mutex_unlock(&lock_mutex_map[lid]);
		std::cout<<"id: "<<id<<"  From NONE to ACQUIRING"<<std::endl;
		ret = cl -> call(lock_protocol::acquire, lid, id, r); 

		pthread_mutex_lock(&lock_mutex_map[lid]);

		std::cout<<"id: "<<id<<"  acquire rpc return ret: "<<ret<<std::endl;

		if(ret == lock_protocol::RETRY)
		{
			//waiting for wake up
			while(lock_map[lid] == ACQUIRING)
			{
				std::cout<<"RETRY waiting lid: "<<lid<<" id:"<<id<<" status:"<<lock_map[lid]<<std::endl;
				pthread_cond_wait(&lock_cond_map[lid], &lock_mutex_map[lid]);
			}
		}else
		{
			if(ret != lock_protocol::OK)
				exit(EXIT_FAILURE);
		}
	}

	lock_map[lid] = LOCKED;
	owner_map[lid] = pthread_self();
	pthread_mutex_unlock(&lock_mutex_map[lid]);
	printf("client acquire all OK!\n");

  return lock_protocol::OK;
}

lock_protocol::status
lock_client_cache::release(lock_protocol::lockid_t lid)
{
	std::cout<<"call release lid"<<lid<<" id:"<<id<<std::endl;
	
	if(lock_map[lid] == FREE) 
		return lock_protocol::OK;

	pthread_mutex_lock(&lock_mutex_map[lid]);

	while(lock_map[lid] != LOCKED){
		std::cout<<"release-- waiting status: "<<lock_map[lid]<<" id:"<<id<<std::endl;
		pthread_cond_wait(&lock_cond_map[lid], &lock_mutex_map[lid]);
	}	
	lock_map[lid] = FREE;
	owner_map[lid] = 0;
	pthread_cond_signal(&lock_cond_map[lid]);
	pthread_mutex_unlock(&lock_mutex_map[lid]);
	printf("release OK!\n");

  return lock_protocol::OK;
}

rlock_protocol::status
lock_client_cache::revoke_handler(lock_protocol::lockid_t lid, 
                                  int &)
{
	int r;
  int ret = rlock_protocol::OK;

	pthread_mutex_lock(&lock_mutex_map[lid]);
	while(lock_map[lid] == ACQUIRING || lock_map[lid] == LOCKED || lock_map[lid] == RELEASING)
	{
		std::cout<<"revoke-- waiting status: "<<lock_map[lid]<<" id:"<<id<<std::endl;
		pthread_cond_wait(&lock_cond_map[lid], &lock_mutex_map[lid]);
	}

	if(lock_map[lid] == NONE)
		return ret;

	lock_map[lid] = RELEASING;
	pthread_mutex_unlock(&lock_mutex_map[lid]);

	std::cout<<"release rpc lid:"<<lid<<" id:"<<id<<std::endl;

	ret = cl -> call(lock_protocol::release, lid, id, r);
	if(ret != lock_protocol::OK)
		exit(EXIT_FAILURE);

	pthread_mutex_lock(&lock_mutex_map[lid]);
	lock_map[lid] = NONE;
	owner_map[lid] = 0;
//Problem?????????????????????????????????????????????
	pthread_cond_signal(&lock_cond_map[lid]);

	pthread_mutex_unlock(&lock_mutex_map[lid]);
	printf("revoke OK!\n");
  return ret;
}

rlock_protocol::status
lock_client_cache::retry_handler(lock_protocol::lockid_t lid, 
                                 int &)
{
  int ret = rlock_protocol::OK;

	pthread_mutex_lock(&lock_mutex_map[lid]);
	while(lock_map[lid] != ACQUIRING){
		std::cout<<"retry waiting --lid: "<<lid<<" id:"<<id<<" status:"<<lock_map[lid]<<std::endl;
		pthread_cond_wait(&lock_cond_map[lid], &lock_mutex_map[lid]);
	}

	std::cout<<"retry middle --lid: "<<lid<<" id:"<<id<<" status:"<<lock_map[lid]<<std::endl;
	if(lock_map[lid] == LOCKED)
		return ret;

	lock_map[lid] = LOCKED;
	pthread_cond_signal(&lock_cond_map[lid]);

	pthread_mutex_unlock(&lock_mutex_map[lid]);
  return ret;
}



