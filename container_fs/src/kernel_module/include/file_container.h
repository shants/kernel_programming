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
//     Header File of Processor Container
//
////////////////////////////////////////////////////////////////////////

#ifndef PROCESSOR_CONTAINER_H
#define PROCESSOR_CONTAINER_H

#include <linux/types.h>

struct file_container_cmd
{
    __u64 op;
    __u64 cid;
    int pid;
};

struct task_struct_node 
{
    struct task_struct *task;
    struct task_struct_node *next_task;
};

struct container_node 
{
    int cid;
    int semaphore;
    struct container_node *next_container;
    struct task_struct_node *task_head;
    struct task_struct_node *task_tail;    
};

#define FCONTAINER_IOCTL_DELETE _IOWR('N', 0x45, struct file_container_cmd)
#define FCONTAINER_IOCTL_CREATE _IOWR('N', 0x46, struct file_container_cmd)
#define FCONTAINER_IOCTL_GETCID _IOWR('N', 0x47, struct file_container_cmd)

#endif
