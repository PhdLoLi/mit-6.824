#ifndef lock_server_cache_h
#define lock_server_cache_h

#include <string>

#include <map>
#include "lock_protocol.h"
#include "rpc.h"
#include "lock_server.h"
#include <list>


class lock_server_cache {
 private:
  int nacquire;
	struct lockinfo
	{
		std::string lock;
		std::string owner;
		std::list<std::string> waiting;
	};
	std::map<lock_protocol::lockid_t, lockinfo> lock_map;
	std::map<lock_protocol::lockid_t, pthread_mutex_t> lock_mutex_map;
	std::map<lock_protocol::lockid_t, pthread_cond_t> lock_cond_map;

 public:
  lock_server_cache();
  lock_protocol::status stat(lock_protocol::lockid_t, int &);
  int acquire(lock_protocol::lockid_t, std::string id, int &);
  int release(lock_protocol::lockid_t, std::string id, int &);
};

#endif
