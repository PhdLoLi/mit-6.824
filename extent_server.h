// this is the extent server

#ifndef extent_server_h
#define extent_server_h

#include <string>
#include <map>
#include "extent_protocol.h"

class extent_server {

 public:
	 std::map<extent_protocol::extentid_t, std::string> extent_map;
	 std::map<extent_protocol::extentid_t, extent_protocol::attr> attr_map;
	 pthread_mutex_t extent_map_m_;
	 pthread_mutex_t attr_map_m_;
  extent_server();

  int put(extent_protocol::extentid_t id, std::string, int &);
  int get(extent_protocol::extentid_t id, std::string &);
  int getattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  int remove(extent_protocol::extentid_t id, int &);
};

#endif 







