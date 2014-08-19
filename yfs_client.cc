// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include "lock_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <time.h>
#include <stdlib.h>
#include "lock_client_cache.h"

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
	ec = new extent_client(extent_dst);
//	ld = new lock_client(lock_dst);
	ld = new lock_client_cache(lock_dst, ec);

}

	yfs_client::inum
yfs_client::n2i(std::string n)
{
	std::istringstream ist(n);
	unsigned long long finum;
	ist >> finum;
	return finum;
}

	std::string
yfs_client::filename(inum inum)
{
	std::ostringstream ost;
	ost << inum;
	return ost.str();
}

	bool
yfs_client::isfile(inum inum)
{
	if(inum & 0x80000000)
		return true;
	return false;
}

	bool
yfs_client::isdir(inum inum)
{
	return ! isfile(inum);
}

	int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the file lock

	printf("getfile %016llx\n", inum);
	extent_protocol::attr a;
	ld -> acquire(inum);
	if (ec->getattr(inum, a) != extent_protocol::OK) {
		r = IOERR;
		goto release;
	}

	fin.atime = a.atime;
	fin.mtime = a.mtime;
	fin.ctime = a.ctime;
	fin.size = a.size;
	printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
	ld -> release(inum);

	return r;
}

	int
yfs_client::getdir(inum inum, dirinfo &din)
{
 	int r = OK;
  // You modify this function for Lab 3
  // - hold and release the directory lock

  printf("getdir %016llx\n", inum);
  extent_protocol::attr a;
	ld -> acquire(inum);
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }
  din.atime = a.atime;
  din.mtime = a.mtime;
  din.ctime = a.ctime;

 release:
	ld -> release(inum);
  return r;
}

int
yfs_client::lookup(inum parent, std::string name, inum &inum, int type, std::string &buf)
{
	std::cout<<"time: now"<<time(0)<<std::endl;
	int r = NOENT;
//	std::string buf;
	printf("yfs_client Here!! lookup parent inum: %016llx\n", parent);

	ld -> acquire(parent);
	int ret = ec->get(parent, buf);

	if (ret != OK)
	{
		printf("yfs_client Here!! lookup geterror!\n");
		ld -> release(parent);
		return ret;
	}
	
	std::cout<<"before ss buf: "<<buf<<std::endl;
	std::stringstream ss(buf);
	std::string sub_str;

	while (getline(ss, sub_str, '|'))
	{
		if(sub_str == name)
		{
			printf("FIND IT!!!\n");
			std::cout<<"sub_str: "<<sub_str<<std::endl;
			getline(ss, sub_str, '|');
			inum = n2i(sub_str);
			r = OK;
			break;
		}
	}

	switch(type)
	{
		case 0: //just lookup 
			{
				ld -> release(parent);
				break;
			}
		case 1://create file
			{
				if(r == OK)
					ld -> release(parent);
				break;
			}
		case 2://mkdir
			{
				if(r == OK)
					ld -> release(parent);
				break;
			}
		case 3://unlink
			{
				if(r == NOENT)
					ld -> release(parent);
				break;
			}
	}

	printf("It's Here check lookup name OVER: ");
	std::cout<<name<<std::endl;
	std::cout<<"buf: "<<buf <<std::endl;

	return r;
}

int
yfs_client::createfile(inum parent, std::string name, inum &inum, std::string parent_buf)
{
	int r = OK;
	std::string buf = "";
	printf("yfs_createfile Here!! parent:  %016llx\n",parent);
	srandom(getpid());
	inum = random() | 0x80000000;	

	//ld -> acquire(inum);
	while(ec->get(inum, buf) == OK)
		inum = random() | 0x80000000;	
	printf("yfs_createfile new inum: %016llx\t",inum);	
	std::cout<<"name: "<<name<<std::endl;
	std::cout<<"parent_buf: "<<parent_buf <<std::endl;

	//create new empty file whose inum is inum;
	
	//ld -> acquire(parent);
	/*
	r = ec -> get(parent, buf);
	if(r != extent_protocol::OK)
	{
		ld -> release(parent);
		return r;
	}
	*/
	r = ec -> put(parent, parent_buf + name + "|" + filename(inum) + "|");
	ld -> release(parent);

	if (r != extent_protocol::OK)
	{
		printf("yfs_createfile add <name|%016llx> error\n",inum);
	}

	ld -> acquire(inum);
	r = ec -> put(inum, "");
	ld -> release(inum);

	return r;
}

int 
yfs_client::mkdir(inum parent, std::string name, inum &inum, std::string parent_buf)
{
	int r = OK;
	std::string buf = "";
	printf("yfs_mkdir Here!! parent:  %016llx\n",parent);
	srandom(getpid());
	inum = random() & 0x7fffffff;	

	//ld -> acquire(inum);
	while(inum == 1 || ec->get(inum, buf) == OK)
//	while(inum ==1 || parent_buf.find(filename(inum)) != parent_buf.npos)
		inum = random() & 0x7fffffff;	
	printf("yfs_mkdir new inum: %016llx\t",inum);	
	std::cout<<"name: "<<name<<std::endl;

	//create new empty dir whose inum is inum;

	//ld -> acquire(parent);
	/*
	r = ec -> get(parent, buf);
	if(r != extent_protocol::OK)
	{
		ld -> release(parent);
		return r;
	}
	*/
	std::cout<<"parent_buf:"<<parent_buf<<std::endl;
	r = ec -> put(parent, parent_buf + name + "|" + filename(inum) + "|");
	ld -> release(parent);

	if (r != extent_protocol::OK)
	{
		printf("yfs_mkdir add <name|%016llx> error\n",inum);
	}

	ld -> acquire(inum);
	r = ec -> put(inum, "");
	ld -> release(inum);
	return r;
}

int
yfs_client::readdir(inum parent, std::vector<dirent> &dirent_vec)
{
	int r;
	std::string buf="";
	printf("yfs_readdir Here!! parent: %016llx\n",parent);
	
	ld -> acquire(parent);
	r = ec -> get(parent,buf);
	ld -> release(parent);

	if(r != OK)
		return r;

	std::stringstream ss(buf);
	std::string sub_str;

	while (getline(ss, sub_str, '|'))
	{
		if(sub_str!="")
		{
			dirent one;
			one.name = sub_str; 
			getline(ss, sub_str, '|');
			one.inum = n2i(sub_str);
			dirent_vec.push_back(one);
		}
	}
	return r;
}

int 
yfs_client::truncate_zero(inum inum)
{
	int r ;
	ld -> acquire(inum);
	r = ec -> put(inum, "");
	ld -> release(inum);
	return r;
}

int 
yfs_client::truncate_size(inum inum, off_t new_size)
{
	int r;
	std::string buf="";
	ld -> acquire(inum);
	r = ec -> get(inum, buf);
	if (r!=OK)
	{
		ld -> release(inum);
		return r;
	}
	buf.resize(new_size, '\0');
	r = ec -> put(inum, buf);
	ld -> release(inum);
	return r;
}

int
yfs_client::read(inum inum, size_t size, off_t off, std::string &buf)
{
	int r;
	buf = "";
	std::string original_buf; 
	off_t original_size;
	ld -> acquire(inum);
	r = ec -> get(inum, original_buf);
	ld -> release(inum);
	original_size = original_buf.size();
	if (r!=OK || original_size <= off)
		return r;
	if( off+size >= original_size )
//		buf = std::string(original_buf.substr(off),original_size-off);
		buf = original_buf.substr(off);
		
	else
	//	buf = std::string(original_buf.substr(off, size),size);
		buf = original_buf.substr(off, size);
	
	return r;
}

int 
yfs_client::write(inum inum, size_t size, off_t off, std::string buf)
{
	int r;
	std::string original_buf;
	off_t original_size;

	ld -> acquire(inum);
	r = ec -> get(inum, original_buf);
	original_size = original_buf.size();

	if(r!=OK)
	{
		ld -> release(inum);
		return r;
	}
//append include '\0'
	if(original_size <= off)
	{
		buf = original_buf + std::string(off-original_size ,'\0') + buf.substr(0, size);
	}else
	{
		if(off + size >= original_size-1)
			buf = original_buf.substr(0, off) + buf.substr(0, size);
		else
			buf = original_buf.substr(0, off) + buf.substr(0,size) + original_buf.substr(off+size);
	}
	std::cout<<"write buf: "<<buf<<std::endl;
	r = ec -> put(inum, buf);
	ld -> release(inum);

	return r;
}

int 
yfs_client::unlink(inum parent, inum inum, std::string name, std::string parent_buf)
{
	int r;
	std::cout<<"unlink name:"<<name<<" inum:"<<inum<<std::endl;
//	std::string buf ="";

 //do with parent

//	ld -> acquire(parent);
/*
	r = ec -> get(parent, buf);
	if(r != extent_protocol::OK)
	{
		ld -> release(parent);
		return r;
	}
	*/
	std::string sub_str = name + "|" + filename(inum) +"|" ;	
	
	std::string::size_type pos = parent_buf.find(sub_str);
	parent_buf.erase(pos, sub_str.size());
	std::cout<<"parent:"<<parent<<" parent_buf:"<<parent_buf<<std::endl;
	r = ec -> put(parent, parent_buf);
	ld -> release(parent);
	if(r != OK)
		return r;
	std::cout<<"parent remove OK, it's me "<<inum<<std::endl;
	ld -> acquire(inum);
	r = ec -> remove(inum);
	ld -> release(inum);

	return r;
}
