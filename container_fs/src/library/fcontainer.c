//////////////////////////////////////////////////////////////////////
//                      North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng, Yu-Chia Liu
//
//   Description:
//     User APIs of Memory Container in User Space
//
////////////////////////////////////////////////////////////////////////

#include "fcontainer.h"

/**
 * delete function in user space that sends command to kernel space
 * for deleting the current task in specified container.
 */
int fcontainer_delete(int devfd)
{
    struct file_container_cmd cmd;
    return ioctl(devfd, FCONTAINER_IOCTL_DELETE, &cmd);
}

/**
 * create function in user space that sends command to kernel space
 * for creating the current task in specified container.
 */
int fcontainer_create(int devfd, int cid)
{
    struct file_container_cmd cmd;
    cmd.cid = cid;
    return ioctl(devfd, FCONTAINER_IOCTL_CREATE, &cmd);
}

/**
 * create function in user space that sends command to kernel space
 * for creating the current task in specified container.
 */
int fcontainer_getcid(int devfd, int pid)
{
    struct file_container_cmd cmd;
    cmd.pid = pid; 
    return ioctl(devfd, FCONTAINER_IOCTL_GETCID, &cmd);
}

