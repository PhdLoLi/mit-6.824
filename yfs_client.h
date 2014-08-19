#ifndef yfs_client_h
#define yfs_client_h

#include <string>
//#include "yfs_protocol.h"
#include "extent_client.h"
#include <vector>

#include "lock_protocol.h"
#include "lock_client.h"
#include "lock_client_cache.h"

class yfs_client {
  extent_client *ec;
	lock_client_cache *ld;
	//yfs_lock_release_user *lu;
 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };
	  
 private:
  static std::string filename(inum);
  static inum n2i(std::string);
 public:

  yfs_client(std::string, std::string);

  bool isfile(inum);
  bool isdir(inum);

  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);
  
  int lookup(inum, std::string, inum &, int, std::string &);
	int createfile(inum, std::string, inum &, std::string);
	int mkdir(inum, std::string, inum &, std::string);
	int readdir(inum, std::vector<dirent> &);
	int truncate_zero(inum);
	int truncate_size(inum, off_t);
	int read(inum, size_t, off_t, std::string &);
	int write(inum, size_t, off_t, std::string);
	int unlink(inum, inum, std::string, std::string);
};

#endif 
