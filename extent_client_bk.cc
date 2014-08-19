// RPC stubs for clients to talk to extent_server

#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// The calls assume that the caller holds a lock on the extent

extent_client::extent_client(std::string dst)
{
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("extent_client: bind failed\n");
  }
}

extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, std::string &buf)
{
  extent_protocol::status ret = extent_protocol::OK;
	if(extent_map.count(eid) == 0)
	{
		ret = cl->call(extent_protocol::get, eid, buf);
		extent_map[eid] = buf;
		dirty_map[eid] = 0;
	}else
	{
		buf = extent_map[eid];
	}
	attr_map[eid].atime = time(0);
  return ret;
}

extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
		       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
	if(attr_map.count(eid) == 0)
	{
		ret = cl->call(extent_protocol::getattr, eid, attr);
		attr_map[eid] = attr;	
	}else
	{
		attr = attr_map[eid];
	}
  return ret;
}

extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, std::string buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  //ret = cl->call(extent_protocol::put, eid, buf, r);
	extent_map[eid] = buf;
	dirty_map[eid] = 1;
	attr_map[eid].mtime = time(0);
	attr_map[eid].ctime = time(0);
	attr_map[eid].size = extent_map[eid].size();
  return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
  extent_protocol::status ret = extent_protocol::OK;
  //ret = cl->call(extent_protocol::remove, eid, r);
	if(extent_map.count(eid) == 0)
		return extent_protocol::IOERR;
	extent_map.erase(eid);
	if(attr_map.count(eid) == 0)
		return extent_protocol::IOERR;
	attr_map.erase(eid);
	dirty[eid] = 2;
  return ret;
}
