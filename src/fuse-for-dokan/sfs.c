/*
** Made by texane <texane@gmail.com>
** 
** Started on  Sun Jun 01 09:27:13 2008 texane
** Last update Tue Jun 03 23:34:29 2008 texane
*/



#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "fuse.h"
#include "debug.h"



static const char *hello_str = "Hello World!\n";

static const char *hello_path = "/hello";

static int sfs_getattr(const char *path, struct stat *stbuf) {

  int res = 0;

  DEBUG_ENTER();

  memset(stbuf, 0, sizeof(struct stat));

  if(strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  }
  else if(strcmp(path, hello_path) == 0) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(hello_str);
  }
  else
    res = -ENOENT;

  return res;
}

static int sfs_access(const char *path, int mask) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_readlink(const char *path, char *buf, size_t size) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi) {

  struct stat stbuf;

  DEBUG_ENTER();

  printf("path: %s\n", path);

  if(strcmp(path, "/") != 0)
    return -ENOENT;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  memset(&stbuf, 0, sizeof(stbuf));
  stbuf.st_size = sizeof(hello_str);
  stbuf.st_nlink = 1;
  stbuf.st_mode = _S_IFREG | _S_IREAD;
  filler(buf, hello_path + 1, &stbuf, 0);

  return 0;
}

static int sfs_mknod(const char *path, mode_t mode, dev_t rdev) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_mkdir(const char *path, mode_t mode) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_symlink(const char *from, const char *to) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_unlink(const char *path) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_rmdir(const char *path) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_rename(const char *from, const char *to) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_link(const char *from, const char *to) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_chmod(const char *path, mode_t mode) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_chown(const char *path, uid_t uid, gid_t gid) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_truncate(const char *path, off_t size) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_utime(const char *path, struct utimbuf *buf) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_open(const char *path, struct fuse_file_info *fi) {

  DEBUG_ENTER();

  printf("open(%s)\n", path);

  if(strcmp(path, hello_path) != 0)
    return -ENOENT;

  if((fi->flags & 3) != O_RDONLY)
    return -EACCES;

  return 0;

}

static int sfs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi) {

  size_t len;

  DEBUG_ENTER();

  if(strcmp(path, hello_path) != 0)
    return -ENOENT;

  len = strlen(hello_str);
  if ((size_t)offset < len) {
    if (offset + size > len)
      size = len - offset;
    memcpy(buf, hello_str + offset, size);
  } else
    size = 0;

  return size;
}

static int sfs_write(const char *path, const char *buf, size_t size, 
			off_t offset, struct fuse_file_info *fi) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_statfs(const char *path, struct statvfs *stbuf) {

  DEBUG_ENTER();

  return 0;
}

static int sfs_release(const char *path, struct fuse_file_info *fi) {
	
  DEBUG_ENTER();

  return 0;
}

static int sfs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {

  DEBUG_ENTER();

  return 0;
}

static void *sfs_init(struct fuse_conn_info *conn) {

  DEBUG_ENTER();

  return NULL;
}

static void sfs_destroy(void *arg) {
  DEBUG_ENTER();
}



/* sfs operations
 */

static struct fuse_operations sfs_oper = {
  sfs_getattr,
  sfs_readlink,
  NULL, /* getdir */
  sfs_mknod,
  sfs_mkdir,
  sfs_unlink,
  sfs_rmdir,
  sfs_symlink,
  sfs_rename,
  sfs_link,
  sfs_chmod,
  sfs_chown,
  sfs_truncate,
  sfs_utime,
  sfs_open,
  sfs_read,
  sfs_write,
  sfs_statfs,
  NULL, /* flush */
  sfs_release,
  sfs_fsync,
  NULL, /* setxattr */
  NULL, /* getxattr */
  NULL, /* sfs_listxattr */
  NULL, /* sfs_removexattr */
  NULL, /* opendir */
  sfs_readdir,
  NULL, /* releasedir */
  NULL, /* fsyncdir */
  sfs_init,
  sfs_destroy,
  sfs_access,
  NULL, /* create */
  NULL, /* ftruncate */
  NULL, /* getattr */
  NULL, /* lock */
  NULL, /* utimens */
  NULL /* bmap */
};



int main(int ac, char** av)
{
    char* fusev[4];

    if (ac != 2) {
      printf("usage: %s <mountpoint>\n", av[0]);
      return -1;
    }

    fusev[0] = av[0];
    fusev[1] = "-o";
    fusev[2] = "nonempty";
    fusev[3] = av[1];

    return fuse_main(ac+1, fusev, &sfs_oper, NULL);
}
