// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <string>
#include "extent_protocol.h"
#include "rpc.h"
#include "lock_client_cache.h"

class extent_client : public lock_release_user {
 private:
  rpcc *cl;
	std::map<extent_protocol::extentid_t, std::string> extent_map;
	std::map<extent_protocol::extentid_t, extent_protocol::attr> attr_map;
	std::map<extent_protocol::extentid_t, int> dirty_map;
//	std::map<extent_protocol::extentid_t, pthread_mutex_t> extent_map_m;
	pthread_mutex_t extent_m_;

 public:
  extent_client(std::string dst);

  extent_protocol::status get(extent_protocol::extentid_t eid, 
			      std::string &buf);
  extent_protocol::status getattr(extent_protocol::extentid_t eid, 
				  extent_protocol::attr &a);
  extent_protocol::status put(extent_protocol::extentid_t eid, std::string buf);
  extent_protocol::status remove(extent_protocol::extentid_t eid);
	extent_protocol::status flush(extent_protocol::extentid_t eid);
	void dorelease(lock_protocol::lockid_t);
};

#endif 

