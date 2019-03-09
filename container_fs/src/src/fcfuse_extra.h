/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/

struct fcfuse_state {
    FILE *logfile;
    char *device_name;
    int devfd;
    char *rootdir;
};

#define FCFS_DATA ((struct fcfuse_state *) fuse_get_context()->private_data)
