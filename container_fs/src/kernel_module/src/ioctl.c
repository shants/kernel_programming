//////////////////////////////////////////////////////////////////////
//                      North Carolina State University
//
//
//
//                             Copyright 2018
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
#include <linux/kthread.h>

extern struct mutex mutex;
extern struct container_node *container_head;
extern struct container_node *container_tail;
extern int semaphore;

struct container_node *find_container(int target_cid)
{
    struct container_node *cursor = container_head;
    while (cursor != NULL)
    {
        if (cursor->cid == target_cid)
        {
            break;
        }
        else
        {
            cursor = cursor->next_container;
        }
    }
    return cursor;
}

/**
 * find what container the current process register.
 */
int file_container_get_container_id(struct file_container_cmd __user *user_cmd)
{
    struct file_container_cmd cmd;
    struct container_node *cursor = container_head;
    struct task_struct_node *task_cursor;
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
        return -1;
    while (cursor != NULL)
    {
        task_cursor = cursor->task_head;
        while(task_cursor)
        {
            // if no task assigned to this task, it means reaching the end of the 
            // task in this container.
            if (task_cursor->task == NULL)
            {
                break;
            }
            // if found the match and the task still register to this container.
            else if(task_cursor->task->pid == cmd.pid)
            {
                return cursor->cid;
            }
            // else go to the next task node.
            else
            {
                task_cursor = task_cursor->next_task;
            }
        }
        cursor = cursor->next_container;
    }
    return -1;
}

/**
 * Remove the task from the container.
 * 
 * external functions needed:
 * mutex_lock(), mutex_unlock() 
 */
int file_container_delete(struct file_container_cmd __user *user_cmd)
{
    struct container_node *container_cursor;
    struct task_struct_node *task_cursor;

    mutex_lock(&mutex);

    if (container_head == NULL)
    {
        printk("No Container in the queue\n");
        mutex_unlock(&mutex);
        return -1;
    }

    if (container_head->task_head == NULL)
    {
        printk("No task in the container\n");
        mutex_unlock(&mutex);
        return -1;
    }

    container_cursor = container_head;
    task_cursor = container_head->task_head;
    container_head->task_head = container_head->task_head->next_task;
    kfree(task_cursor);

    printk("TID: %d Container: %d Switched Delete\n", current->pid, container_cursor->cid);

    // if the container doesn't have any task, we can destroy this container
    if (container_cursor->task_head == NULL)
    {
        printk("free the container %d\n", container_cursor->cid);
        container_head = container_head->next_container;
        kfree(container_cursor);
    }

    // else, we move this container to the end of container queue because this container has taken actions.
    else
    {
        // if more than one container left
        if (container_head->next_container != NULL)
        {
            container_head = container_head->next_container;
            container_tail->next_container = container_cursor;
            container_tail = container_cursor;
            container_cursor->next_container = NULL;
        }
    }

    mutex_unlock(&mutex);

    return 0;
}


/**
 * Create/Assign a task in the corresponding container.
 * external functions needed:
 * 
 */

int file_container_create(struct file_container_cmd __user *user_cmd)
{
    struct file_container_cmd cmd;
    struct container_node *container_cursor;
    struct task_struct_node *task_cursor;
    int runme = 0;

    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
        return -1;

    mutex_lock(&mutex);

    // find whether the container is created or not first
    container_cursor = find_container(cmd.cid);

    // if it is NULL, create a new one and put it into container queue
    if (container_cursor == NULL)
    {
        container_cursor = (struct container_node *)kcalloc(1, sizeof(struct container_node), GFP_KERNEL);
        container_cursor->cid = cmd.cid;
        printk("create a new container %d\n", container_cursor->cid);

        // if nothing in the container queue.
        if (container_head == NULL)
        {
            container_head = container_cursor;
        }
        else
        {
            container_tail->next_container = container_cursor;
        }
        container_tail = container_cursor;
    }

    // put the new task to container
    task_cursor = (struct task_struct_node *)kcalloc(1, sizeof(struct task_struct_node), GFP_KERNEL);
    task_cursor->task = current;

    // if the task queue in the container is empty
    if (container_cursor->task_head == NULL)
    {
        container_cursor->task_head = task_cursor;
    }

    // else, append the task to the tail of queue.
    else
    {
        container_cursor->task_tail->next_task = task_cursor;
    }
    container_cursor->task_tail = task_cursor;

    printk("create a new task %d in container %d\n", task_cursor->task->pid, container_cursor->cid);
//    print_current_queue();

    mutex_unlock(&mutex);

//    printk("TID: %d Container: %d Started\n", current->pid, container_cursor->cid);
    return 0;
}


/**
 * control function that receive the command in user space and pass arguments to
 * corresponding functions.
 */
int file_container_ioctl(struct file *filp, unsigned int cmd,
                              unsigned long arg)
{
    switch (cmd)
    {
    case FCONTAINER_IOCTL_CREATE:
        return file_container_create((void __user *)arg);
    case FCONTAINER_IOCTL_GETCID:
        return file_container_get_container_id((void __user *)arg);
    case FCONTAINER_IOCTL_DELETE:
        return file_container_delete((void __user *)arg);
    default:
        return -ENOTTY;
    }
}
