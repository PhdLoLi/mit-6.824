// the caching lock server implementation

#include "lock_server_cache.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "lang/verify.h"
#include "handle.h"
#include "tprintf.h"


lock_server_cache::lock_server_cache()
{
}


int lock_server_cache::acquire(lock_protocol::lockid_t lid, std::string id, 
                               int &)
{
	int r;
  lock_protocol::status ret = lock_protocol::OK;
	pthread_mutex_t mutex_temp;
	pthread_cond_t cond_temp;

	if (pthread_mutex_init(&mutex_temp, NULL) != 0 ){               
		perror("Mutex Initialization Failed");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&cond_temp, NULL) != 0 ){               
		perror("Cond Initialization Failed");
		exit(EXIT_FAILURE);
	}

	lock_mutex_map.insert(make_pair(lid, mutex_temp));
	lock_cond_map.insert(make_pair(lid, cond_temp));

	pthread_mutex_lock(&lock_mutex_map[lid]);

	lockinfo one;
	one.lock ="free";
//	one.owner = "";
//	one.waiting = NULL;

	lock_map.insert(make_pair(lid, one));
	std::cout<<"acquire-- lid: "<<lid<<" status: "<<lock_map[lid].lock<<" owner: "<<lock_map[lid].owner<<std::endl;
	if(lock_map[lid].lock == "locked")
	{
		//revoke(one.owner)
		while(lock_map[lid].owner == ""){
			std::cout<<"waiting for new owner "<<std::endl;
			pthread_cond_wait(&lock_cond_map[lid], &lock_mutex_map[lid]);
		}
		std::string ownertemp = lock_map[lid].owner;
		lock_map[lid].owner ="";
		handle h(ownertemp);

		lock_map[lid].waiting.push_back(id);
		std::cout<<"acquire lid:"<<lid<<" waiting push_back" << id<<std::endl;

		pthread_mutex_unlock(&lock_mutex_map[lid]);

		rpcc *cl = h.safebind();
		if(cl){
			std::cout<<"rpc call revoke-- lid:"<<lid<<" owner:" << ownertemp<<std::endl;
			ret = cl -> call(rlock_protocol::revoke, lid, r);
		}else{
			exit(EXIT_FAILURE);
		}
		if(ret != rlock_protocol::OK){
			exit(EXIT_FAILURE);
		}
		
		ret = lock_protocol::RETRY;
	}else{
		printf("Free Original!\n");
		lock_map[lid].lock = "locked";
		lock_map[lid].owner = id;
		pthread_mutex_unlock(&lock_mutex_map[lid]);
	}
	
  
	return ret;
}

int 
lock_server_cache::release(lock_protocol::lockid_t lid, std::string id, 
         int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	if(lock_map[lid].owner == id || lock_map[lid].owner =="")
	{
		std::cout<<"server release-- lid: "<<lid<<"id: "<<id<<std::endl;

		pthread_mutex_lock(&lock_mutex_map[lid]);

		if(lock_map[lid].waiting.size() > 0){
			std::string retryid = lock_map[lid].waiting.front();
			pthread_mutex_unlock(&lock_mutex_map[lid]);

			handle h(retryid);
			rpcc *cl = h.safebind();
			if(cl){
				std::cout<<"retry rpc: retryid"<<retryid<<" lid:"<<lid<<std::endl;
				ret = cl -> call(rlock_protocol::retry, lid, r);
			}else{
				exit(EXIT_FAILURE);
			}
			if(ret != rlock_protocol::OK){
				exit(EXIT_FAILURE);
			}
			
			pthread_mutex_lock(&lock_mutex_map[lid]);
			
			lock_map[lid].owner = retryid;
			lock_map[lid].waiting.pop_front();
			lock_map[lid].lock = "locked";
			pthread_cond_signal(&lock_cond_map[lid]);
		}else{
			printf("relase no waiting\n");
			lock_map[lid].lock = "free";
			lock_map[lid].owner = "";
		}	

		pthread_mutex_unlock(&lock_mutex_map[lid]);
	}	
  return ret;
}

lock_protocol::status
lock_server_cache::stat(lock_protocol::lockid_t lid, int &r)
{
  tprintf("stat request\n");
  r = nacquire;
  return lock_protocol::OK;
}

