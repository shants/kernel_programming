/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  There are a couple of symbols that need to be #defined before
  #including all the headers.
*/

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 29
#define FUSE_USE_VERSION 29

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

#include <limits.h>
#include <stdio.h>

#define NPHFS_DATA ((struct fcfuse_state *) fuse_get_context()->private_data)
#include "fcfuse_extra.h"
#endif

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"
int fcfuse_getattr(const char *path, struct stat *statbuf);
int fcfuse_readlink(const char *path, char *link, size_t size);
int fcfuse_mknod(const char *path, mode_t mode, dev_t dev);
int fcfuse_mkdir(const char *path, mode_t mode);
int fcfuse_unlink(const char *path);
int fcfuse_rmdir(const char *path);
int fcfuse_symlink(const char *path, const char *link);
int fcfuse_rename(const char *path, const char *newpath);
int fcfuse_link(const char *path, const char *newpath);
int fcfuse_chmod(const char *path, mode_t mode);
int fcfuse_chown(const char *path, uid_t uid, gid_t gid);
int fcfuse_truncate(const char *path, off_t newsize);
int fcfuse_utime(const char *path, struct utimbuf *ubuf);
int fcfuse_open(const char *path, struct fuse_file_info *fi);
int fcfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int fcfuse_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi);
int fcfuse_statfs(const char *path, struct statvfs *statv);
int fcfuse_flush(const char *path, struct fuse_file_info *fi);
int fcfuse_release(const char *path, struct fuse_file_info *fi);
int fcfuse_fsync(const char *path, int datasync, struct fuse_file_info *fi);
#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int fcfuse_setxattr(const char *path, const char *name, const char *value, size_t size, int flags);
int fcfuse_getxattr(const char *path, const char *name, char *value, size_t size);
int fcfuse_listxattr(const char *path, char *list, size_t size);
int fcfuse_removexattr(const char *path, const char *name);
#endif
int fcfuse_opendir(const char *path, struct fuse_file_info *fi);
int fcfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int fcfuse_releasedir(const char *path, struct fuse_file_info *fi);
int fcfuse_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi);
void *fcfuse_init(struct fuse_conn_info *conn);
void fcfuse_destroy(void *userdata);
int fcfuse_access(const char *path, int mask);
int fcfuse_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi);
int fcfuse_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi);
