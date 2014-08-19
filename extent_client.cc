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
	VERIFY(pthread_mutex_init(&extent_m_, 0) == 0);
}

extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, std::string &buf)
{
//	ScopedLock rwl(&extent_m_);
  extent_protocol::status ret = extent_protocol::OK;
	if(extent_map.count(eid) == 0)
	{
		ret = cl->call(extent_protocol::get, eid, buf);
		pthread_mutex_lock(&extent_m_);
		extent_map[eid] = buf;
		pthread_mutex_unlock(&extent_m_);
		dirty_map[eid] = 0;
	}else
	{
		buf = extent_map[eid];
	}
	attr_map[eid].atime = time(0);
//	attr_map[eid].size = buf.size();
  return ret;
}

extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
		       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
	if(attr_map.count(eid) == 0 || dirty_map.count(eid) == 0)
	{
		ret = cl->call(extent_protocol::getattr, eid, attr);
		if(ret!=extent_protocol::OK)
			return ret;
		attr_map[eid].size = attr.size;	
		attr_map[eid].atime = attr.atime;	
		attr_map[eid].ctime = attr.ctime;	
		attr_map[eid].mtime = attr.mtime;	
	}else 
	{
		if(dirty_map[eid] ==1)
		{
			attr.size = attr_map[eid].size;
			attr.atime = attr_map[eid].atime;
			attr.ctime = attr_map[eid].ctime;
			attr.mtime = attr_map[eid].mtime;
		}else
		{
			ret = cl->call(extent_protocol::getattr, eid, attr);
			if(ret!=extent_protocol::OK)
				return ret;
			attr_map[eid].size = attr.size;	
			attr_map[eid].ctime = attr.ctime;	
			attr_map[eid].mtime = attr.mtime;	
			attr.atime = attr_map[eid].atime;
		}
	}
  return ret;
}

extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, std::string buf)
{
//	ScopedLock rwl(&extent_m_);
  extent_protocol::status ret = extent_protocol::OK;
  //ret = cl->call(extent_protocol::put, eid, buf, r);
//	extent_map[eid] = buf;
	pthread_mutex_lock(&extent_m_);
	extent_map[eid] = buf;
	pthread_mutex_unlock(&extent_m_);
	dirty_map[eid] = 1;
	attr_map[eid].mtime = time(0);
	attr_map[eid].ctime = time(0);
	attr_map[eid].size = extent_map[eid].size();
  return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
//	ScopedLock rwl(&extent_m_);
  extent_protocol::status ret = extent_protocol::OK;
  //ret = cl->call(extent_protocol::remove, eid, r);
	if(extent_map.count(eid) != 0)
	{
		pthread_mutex_lock(&extent_m_);
		extent_map.erase(eid);
		pthread_mutex_unlock(&extent_m_);
	}
	if(attr_map.count(eid) != 0)
		attr_map.erase(eid);
	dirty_map[eid] = 2;
  return ret;
}

extent_protocol::status
extent_client::flush(extent_protocol::extentid_t eid)
{
//	ScopedLock rwl(&extent_m_);
  extent_protocol::status ret = extent_protocol::OK;
	int r;
	if(dirty_map.count(eid) ==0 )
		return extent_protocol::IOERR;
	std::cout<<"extent_client -- flush here! eid"<<eid<<"dirty tag: "<<dirty_map[eid]<<" buf: "<<extent_map[eid]<<std::endl;
	switch(dirty_map[eid])
	{
		case 0://clean
			{
				pthread_mutex_lock(&extent_m_);
				extent_map.erase(eid);
				attr_map.erase(eid);
				pthread_mutex_unlock(&extent_m_);
				break;
			}
		case 1://dirty
			{	
				std::cout<<"dirty-- eid: "<<eid<<" buf:"<<extent_map[eid]<<std::endl;
				ret = cl->call(extent_protocol::put, eid, extent_map[eid], r);
				if (ret != extent_protocol::OK)
					return ret;
			//	ret = cl->call(extent_protocol::getattr, eid, attr_map[eid]);
				pthread_mutex_lock(&extent_m_);
				extent_map.erase(eid);
				attr_map.erase(eid);
				pthread_mutex_unlock(&extent_m_);
				break;
			}
		case 2://remove
			{
				ret = cl->call(extent_protocol::remove, eid, r);
				break;
			}
	}
	
	dirty_map.erase(eid);
	return ret;
}

void
extent_client::dorelease(lock_protocol::lockid_t lid)
{
	extent_protocol::extentid_t eid = lid;
	flush(eid);
}
