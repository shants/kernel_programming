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
//     Core of Kernel Module for Processor Container
//
////////////////////////////////////////////////////////////////////////

#include "file_container.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/sched.h>

extern struct miscdevice file_container_dev;

struct mutex mutex;
struct container_node *container_head;
struct container_node *container_tail;
int semaphore = 0;

/**
 * Initialize and register the kernel module
 */
int file_container_init(void)
{
    int ret;
    if ((ret = misc_register(&file_container_dev)))
        printk(KERN_ERR "Unable to register \"file_container\" misc device\n");
    else
        printk(KERN_ERR "\"file_container\" misc device installed\n");
        printk(KERN_ERR "\"file_container\" version 2.3\n");
    mutex_init(&mutex);
    return ret;
}


/**
 * Cleanup and deregister the kernel module
 */ 
void file_container_exit(void)
{
    struct container_node *container_cursor;
    struct task_struct_node *task_cursor;
    while (container_head != NULL)
    {
        container_cursor = container_head;
        container_head = container_head->next_container;
        while (container_cursor->task_head != NULL) {
            task_cursor = container_cursor->task_head;
            container_cursor->task_head = container_cursor->task_head->next_task;
            kfree(task_cursor);
        }
        kfree(container_cursor);
    }
    misc_deregister(&file_container_dev);
}
