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

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
	ec = new extent_client(extent_dst);
	ld = new lock_client(lock_dst);

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
//	ld -> acquire(inum);
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
//	ld -> release(inum);

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
//	ld -> acquire(inum);
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }
  din.atime = a.atime;
  din.mtime = a.mtime;
  din.ctime = a.ctime;

 release:
//	ld -> release(inum);
  return r;
}

int
yfs_client::lookup(inum parent, std::string name, inum &inum)
{
	int r = NOENT;
	std::string buf;
	printf("yfs_client Here!! lookup %016llx\n", parent);

//	ld -> acquire(parent);
	int ret = ec->get(parent, buf);
//	ld -> release(parent);

	if (ret != OK)
	{
		printf("yfs_client Here!! lookup geterror!\n");
		return ret;
	}
	
	
	printf("It's Here check lookup name: ");
	std::cout<<name<<std::endl;

	std::stringstream ss(buf);
	std::string sub_str;

	//std::cout<<"buf: "<<buf<<std::endl;

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
	return r;
}

int
yfs_client::createfile(inum parent, std::string name, inum &inum)
{
	int r = OK;
	std::string buf = "";
	printf("yfs_createfile Here!! parent:  %016llx\n",parent);
	srandom(getpid());
	inum = random() | 0x80000000;	

//	ld -> acquire(inum);
	while(ec->get(inum, buf) == OK)
		inum = random() | 0x80000000;	
	printf("yfs_createfile new inum: %016llx\t",inum);	
	std::cout<<"name: "<<name<<std::endl;
	r = ec -> put(inum, "");
//	ld -> release(inum);

	//create new empty file whose inum is inum;
	std::cout<<"ec->put r:"<<r<<std::endl;
	if (r != extent_protocol::OK)
	{
		printf("yfs_createfile create new empty file error!\n");
		return r;
	}
	
//	ld -> acquire(parent);
	r = ec -> get(parent, buf);
	if(r != extent_protocol::OK)
	{
//		ld -> release(parent);
		return r;
	}
	r = ec -> put(parent, buf + name + "|" + filename(inum) + "|");
//	ld -> release(parent);

	if (r != extent_protocol::OK)
	{
		printf("yfs_createfile add <name|%016llx> error\n",inum);
	}
	return r;
}

int 
yfs_client::mkdir(inum parent, std::string name, inum &inum)
{
	int r = OK;
	std::string buf = "";
	printf("yfs_mkdir Here!! parent:  %016llx\n",parent);
	srandom(getpid());
	inum = random() & 0x7fffffff;	

	//ld -> acquire(inum);
	while(inum == 1 || ec->get(inum, buf) == OK)
		inum = random() & 0x7fffffff;	
	printf("yfs_mkdir new inum: %016llx\t",inum);	
	std::cout<<"name: "<<name<<std::endl;
	r = ec -> put(inum, "");
//	ld -> release(inum);

	//create new empty dir whose inum is inum;
	std::cout<<"ec->put r:"<<r<<std::endl;
	if (r != extent_protocol::OK)
	{
		printf("yfs_mkdir create new empty dir error!\n");
		return r;
	}

//	ld -> acquire(parent);
	r = ec -> get(parent, buf);
	if(r != extent_protocol::OK)
	{
//		ld -> release(parent);
		return r;
	}
	r = ec -> put(parent, buf + name + "|" + filename(inum) + "|");
//	ld -> release(parent);

	if (r != extent_protocol::OK)
	{
		printf("yfs_mkdir add <name|%016llx> error\n",inum);
	}
	return r;
}

int
yfs_client::readdir(inum parent, std::vector<dirent> &dirent_vec)
{
	int r;
	std::string buf="";
	printf("yfs_readdir Here!! parent: %016llx\n",parent);
	
//	ld -> acquire(parent);
	r = ec -> get(parent,buf);
//	ld -> release(parent);

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
//	ld -> acquire(inum);
	r = ec -> put(inum, "");
//	ld -> release(inum);
	return r;
}

int 
yfs_client::truncate_size(inum inum, off_t new_size)
{
	int r;
	std::string buf="";
//	ld -> acquire(inum);
	r = ec -> get(inum, buf);
	if (r!=OK)
	{
//		ld -> release(inum);
		return r;
	}
	buf.resize(new_size, '\0');
//	r = ec -> put(inum, std::string(buf, new_size));
	r = ec -> put(inum, buf);
//	ld -> release(inum);
	return r;
}

int
yfs_client::read(inum inum, size_t size, off_t off, std::string &buf)
{
	int r;
	buf = "";
	std::string original_buf; 
	off_t original_size;
//	ld -> acquire(inum);
	r = ec -> get(inum, original_buf);
//	ld -> release(inum);
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

//	ld -> acquire(inum);
	r = ec -> get(inum, original_buf);
	original_size = original_buf.size();

	if(r!=OK)
	{
//		ld -> release(inum);
		return r;
	}
//append include '\0'
	if(original_size <= off)
	{
//		buf = std::string(original_buf + std::string(off-original_size ,'\0') + buf.substr(0, size),size+off+1);
		buf = original_buf + std::string(off-original_size ,'\0') + buf.substr(0, size);
	}else
	{
		if(off + size >= original_size-1)
			buf = original_buf.substr(0, off) + buf.substr(0, size);
		else
			buf = original_buf.substr(0, off) + buf.substr(0,size) + original_buf.substr(off+size);
	}

	r = ec -> put(inum, buf);
//	ld -> release(inum);

	return r;
}

int 
yfs_client::unlink(inum parent, inum inum, std::string name)
{
	int r;
	std::string buf ="";
//	ld -> acquire(inum);
	r = ec -> remove(inum);
//	ld -> release(inum);

	if(r != OK)
	return r;
 //do with parent

//	ld -> acquire(parent);
	r = ec -> get(parent, buf);
	if(r != extent_protocol::OK)
	{
//		ld -> release(parent);
		return r;
	}
	std::string sub_str = name + "|" + filename(inum) +"|" ;	
	
	std::string::size_type pos = buf.find(sub_str);
	buf.erase(pos, sub_str.size());
	r = ec -> put(parent, buf);
	//ld -> release(parent);
	return r;
}
