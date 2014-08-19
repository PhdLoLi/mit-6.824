// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <time.h>

extent_server::extent_server() {
VERIFY((pthread_mutex_init(&extent_map_m_, 0) == 0));
VERIFY((pthread_mutex_init(&attr_map_m_, 0) == 0));
extent_map[1] = "";
}


int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{
  // You fill this in for Lab 2.
	ScopedLock rwl(&extent_map_m_);
/*
	if(id & 0x80000000) //is file? all overwrite
		extent_map[id] = buf;
	else
		extent_map[id] += buf;
*/
	extent_map[id] = buf;
	printf("put-- id:%016llx \n",id);
	std::cout<<"buf:"<<extent_map[id]<<"time: "<<time(0)<<std::endl;
//	std::cout<<"put-- buf: "<<buf<<std::endl;
	
//	ScopedLock rwl(&attr_map_m_);
	attr_map[id].ctime = time(0);
	attr_map[id].mtime = time(0);
	attr_map[id].size = extent_map[id].size();

  return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{
  // You fill this in for Lab 2.
	ScopedLock rwl(&extent_map_m_);
	if(extent_map.count(id) == 0 && id>1)
	{
		printf("get error-- id:%016llx\textent_server get count(id)==0\n",id);
		return extent_protocol::IOERR;
	}
	printf("get-- id:%016llx\n",id);
	std::cout<<"buf: "<<extent_map[id]<<"time "<<time(0)<<std::endl;
//	std::cout<<"get-- buf  "<<extent_map[id]<<std::endl;
	buf = extent_map[id];

//	ScopedLock rwl(&attr_map_m_);
	attr_map[id].atime = time(0);
  return extent_protocol::OK;
}

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{
  // You fill this in for Lab 2.
  // You replace this with a real implementation. We send a phony response
  // for now because it's difficult to get FUSE to do anything (including
  // unmount) if getattr fails.
	ScopedLock rwl(&attr_map_m_);
	if(attr_map.count(id) == 0 && id>1)
	{
		printf("getattr error-- id:%016llx\n",id);
		return extent_protocol::IOERR;
	}
  a.size = attr_map[id].size;
  a.atime = attr_map[id].atime;
  a.mtime = attr_map[id].mtime;
  a.ctime = attr_map[id].ctime;

  return extent_protocol::OK;
}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{
  // You fill this in for Lab 2.
	ScopedLock rwl(&extent_map_m_);
	if(extent_map.count(id) == 0)
		return extent_protocol::IOERR;
	extent_map.erase(id);

//	ScopedLock rwl(&attr_map_m_);
	if(attr_map.count(id) == 0)
		return extent_protocol::IOERR;
	attr_map.erase(id);

  return extent_protocol::OK;
}

