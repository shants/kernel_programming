/*
  Key Value System
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h
  
  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

*/

#include "fcfuse.h"
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
//#define HAVE_SYS_XATTR_H 1
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

struct fcfuse_state *fcfuse_data;

struct fuse_operations fcfuse_oper = {
  .getattr = fcfuse_getattr,
  .readlink = fcfuse_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = fcfuse_mknod,
  .mkdir = fcfuse_mkdir,
  .unlink = fcfuse_unlink,
  .rmdir = fcfuse_rmdir,
  .symlink = fcfuse_symlink,
  .rename = fcfuse_rename,
  .link = fcfuse_link,
  .chmod = fcfuse_chmod,
  .chown = fcfuse_chown,
  .truncate = fcfuse_truncate,
  .utime = fcfuse_utime,
  .open = fcfuse_open,
  .read = fcfuse_read,
  .write = fcfuse_write,
  .statfs = fcfuse_statfs,
  /** Just a placeholder, don't set */ // huh???
  .flush = fcfuse_flush,
  .release = fcfuse_release,
  .fsync = fcfuse_fsync,
  
#ifdef HAVE_SYS_XATTR_H
  .setxattr = fcfuse_setxattr,
  .getxattr = fcfuse_getxattr,
  .listxattr = fcfuse_listxattr,
  .removexattr = fcfuse_removexattr,
#endif
  
  .opendir = fcfuse_opendir,
  .readdir = fcfuse_readdir,
  .releasedir = fcfuse_releasedir,
  .fsyncdir = fcfuse_fsyncdir,
  .init = fcfuse_init,
  .destroy = fcfuse_destroy,
  .access = fcfuse_access,
  .ftruncate = fcfuse_ftruncate,
  .fgetattr = fcfuse_fgetattr
};


void fcfuse_usage()
{
    fprintf(stderr, "usage:  fcfuse [FUSE and mount options] npheap_device_name dataLocation mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;

    // NPHeapFS doesn't do any access checking on its own (the comment
    // blocks in fuse.h mention some of the functions that need
    // accesses checked -- but note there are other functions, like
    // chown(), that also need checking!).  Since running NPHeapFS as root
    // will therefore open Metrodome-sized holes in the system
    // security, we'll check if root is trying to mount the filesystem
    // and refuse if it is.  The somewhat smaller hole of an ordinary
    // user doing it with the allow_other flag is still there because
    // I don't want to parse the options string.
    if ((getuid() == 0) || (geteuid() == 0)) {
	fprintf(stderr, "Running NPHeapFS as root opens unnacceptable security holes\n");
	return 1;
    }

    // See which version of fuse we're running
    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    
    // Perform some sanity checking on the command line:  make sure
    // there are enough arguments, and that neither of the last two
    // start with a hyphen (this will break if you actually have a
    // rootpoint or mountpoint whose name starts with a hyphen, but so
    // will a zillion other programs)
    if ((argc < 4) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	fcfuse_usage();

    fcfuse_data = (struct fcfuse_state *)malloc(sizeof(struct fcfuse_state));
    if (fcfuse_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the rootdir out of the argument list and save it in my
    // internal data
    fcfuse_data->device_name = (char *)malloc((strlen(argv[argc-3])+1)*sizeof(char));
    strcpy(fcfuse_data->device_name,argv[argc-3]);
    fcfuse_data->devfd = open(fcfuse_data->device_name,O_RDWR);
    fcfuse_data->rootdir = realpath(argv[argc-2], NULL);
    argv[argc-3] = argv[argc-1];
    argv[argc-1] = NULL;
    argv[argc-2] = NULL;
    argc-=2;
    // You can output to a log file for debugging if you would like to.
    fcfuse_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main %s\n",fcfuse_data->rootdir);
    fuse_stat = fuse_main(argc, argv, &fcfuse_oper, fcfuse_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
