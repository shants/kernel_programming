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

#include "memory_container.h"

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


#define MY_PRINT(x,y) printk("MY_PRINT:  (%s):%d -->  %s %d\n",__FUNCTION__,__LINE__,x,y)
#define MY_PRINT_MESG(x) printk("MY_PRINT_MESG:  (%s):%d -->  %s \n",__FUNCTION__,__LINE__,x)
#define MY_PRINT_LU(x,y) printk("MY_PRINT_MESG:  (%s):%d -->  %s %lu\n",__FUNCTION__,__LINE__,x,y)
#define MY_PRINT_LLU(x,y) printk("MY_PRINT_MESG:  (%s):%d -->  %s %llu\n",__FUNCTION__,__LINE__,x,y)

struct memNode
{
    __u64 oid;
    char* memory;
    struct memNode* next;
};

struct task 
{
    struct task_struct* thr;
    struct task* next;
};


struct container
{
    __u64 cid;
    int task_cnt;
    struct task*  task_list;
    struct memNode* mem_list;
    struct container* next;
    struct mutex lock;

};

static struct container* ctr_list = NULL;
DEFINE_MUTEX(my_mutex);


void printObjects(struct container* ctr) {
    struct memNode* mm = NULL;
    if (ctr == NULL){
        MY_PRINT_MESG("ctr is NULL");
    }

    MY_PRINT_MESG("--> START Iterating CTR and MM_LIST");
    for(mm= ctr->mem_list; mm!=NULL; mm=mm->next){
        printk("mm entry %llu, memobj id %llu,mm= %p mm->memory= %p value %d\n", ctr->cid, mm->oid, mm, mm->memory, *(mm->memory));
    }
    MY_PRINT_MESG("--> END Iterating CTR and MMLIST");
}

struct container* getContainerFromCid(__u64 cid){
    struct container* ctrNode ;
    if (ctr_list == NULL) {
        return NULL;
    }
    ctrNode = ctr_list;
    while(ctrNode != NULL){
        if ( ctrNode->cid == cid ) {
            printk( "Container Found %llu \n",ctrNode->cid);    
            break;
        }
        ctrNode = ctrNode->next;
    }
    return ctrNode;
}

struct container* getCurrentContainer(void){
    struct container* ctrNode = NULL;
    struct task* temp = NULL;
    if (ctr_list == NULL) {
        return NULL;
    }

    MY_PRINT("Get container for task ", current->pid);
    ctrNode = ctr_list;
    while(ctrNode != NULL){
        temp= ctrNode->task_list;
        while(temp != NULL) {
            if (temp->thr == current){
                MY_PRINT("Container found for task ", temp->thr->pid);
                //MY_PRINT_LLU("Container id found for task ", ctrNode->cid);
                return ctrNode;
            }
            temp = temp->next;
        }
        ctrNode = ctrNode->next;
    }
    return ctrNode;
}



struct task* getNewTask(void) {
    struct task* tn = NULL;
    tn = (struct task*)kmalloc(sizeof(struct task), GFP_KERNEL);
    if ( tn == NULL){
        MY_PRINT_MESG(" kmalloc failed");
        return NULL;
    }
    tn->next = NULL;
    tn->thr= current; 
    return tn;
}


struct container* getNewContainer(__u64 cid) {
    struct container* ctrNode = NULL;
    ctrNode = (struct container*)kmalloc(sizeof(struct container), GFP_KERNEL);
    if ( ctrNode == NULL){
        MY_PRINT_MESG(" kmalloc failed");
        return NULL;
    }
    ctrNode->task_cnt = 0;
    ctrNode->cid = cid;
    ctrNode->next = NULL;
    ctrNode->task_list = NULL;
    ctrNode->mem_list = NULL;
    mutex_init(&ctrNode->lock);
    return ctrNode;
}


int memory_container_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long offset = 0;
    unsigned long off =  0 ;
    unsigned long len = 0 ;
    
    
    struct container* ctrNode = NULL;
    struct memNode* mm = NULL;
    int ret = 0;
    mutex_lock(&my_mutex);
    //loop into the mem_list and check if the offset is allocated or not
    MY_PRINT_MESG(" mmap called");

    offset = vma->vm_pgoff;

    off = offset;
    len = vma->vm_end - vma->vm_start;

    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED))
    {
        MY_PRINT_MESG("offset should be writable");
        return(-EINVAL);
    }
    vma->vm_flags |= VM_LOCKED;
    
    
    ctrNode = getCurrentContainer();
    if (ctrNode == NULL){
        mutex_unlock(&my_mutex);
        return 0;
    }

    //printObjects(ctrNode);
    //printk("now mmap objid %lu to ctrid %llu\n",off,ctrNode->cid);
    //MY_PRINT_MESG("A");
    //printk("mem_list %p \n",ctrNode->mem_list);
    
    mm = ctrNode->mem_list;
    //MY_PRINT_MESG("B");
    while(mm != NULL){
            //printk("mmNode loop %llu for objid %llu \n", ctrNode->cid, mm->oid);
            if (mm->oid == off) {
                //printk("mmNode found ctr %llu for objid %llu \n", ctrNode->cid, mm->oid);
                break;
            }else{
                mm=mm->next;
            }
    }
    if (mm == NULL){
        //printk("mmNode Not found ctr %llu for objid %lu \n", ctrNode->cid, off);
        //printk(" Allocating  \n");
        mm = (struct memNode*)kmalloc(sizeof(struct memNode), GFP_KERNEL);//add start
        memset(mm,'\0',sizeof(struct memNode));
        if (mm == NULL){
            MY_PRINT_MESG(" kmalloc failed");
            mutex_unlock(&my_mutex);
            return -1;
        }else{
            mm->oid= off;
            mm->memory= (char*)kmalloc(len, GFP_KERNEL);
            if (mm->memory == NULL){
                MY_PRINT_MESG(" kmalloc failed");
                mutex_unlock(&my_mutex);
                return -1;
            }
            memset(mm->memory, '\0', len);
            mm->next = NULL;
            if (ctrNode->mem_list == NULL){
                ctrNode->mem_list = mm;
            }else{
                mm->next=ctrNode->mem_list;
                ctrNode->mem_list = mm;
            }
        }
    }
    
    //MY_PRINT_MESG("Container State before remap ");
    //printObjects(ctrNode);
    
    //MY_PRINT_MESG(" remap ");
    if (mm->memory !=NULL){
        ret = remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)mm->memory) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
        if (ret < 0 ){
            mutex_unlock(&my_mutex);
            return -EIO;
        }
        //MY_PRINT_MESG(" return ");
    }

    mutex_unlock(&my_mutex);
    return 0;
}


int memory_container_lock(struct memory_container_cmd __user *user_cmd)
{
    struct memory_container_cmd ctrCmd ;
    struct container* ctrNode = NULL;
    __u64 oid;
    mutex_lock(&my_mutex);
    memset(&ctrCmd,'\0',sizeof(ctrCmd));
    copy_from_user(&ctrCmd, user_cmd, sizeof(struct memory_container_cmd));
    oid =  ctrCmd.oid;
    
   
    //MY_PRINT(" get container for current task", current->pid);
    ctrNode = getCurrentContainer();
    //MY_PRINT(" after get container for current task", current->pid);
   
    if(ctrNode == NULL) {
        MY_PRINT_MESG(" container not found return ");
        mutex_unlock(&my_mutex);
        // no container has this task , unlock return
        return 0;
    }
    
    //MY_PRINT(" release global ", current->pid);
    mutex_unlock(&my_mutex);
    //MY_PRINT(" take local lock", current->pid);
    mutex_lock(&ctrNode->lock);    
   
    //MY_PRINT_MESG(" return  ");
    return 0;
}


int memory_container_unlock(struct memory_container_cmd __user *user_cmd)
{
    struct memory_container_cmd ctrCmd ;
    struct container* ctrNode = NULL;
    //struct memNode* mm = NULL;
    __u64 oid;
    mutex_lock(&my_mutex);
    memset(&ctrCmd,'\0',sizeof(ctrCmd));
    copy_from_user(&ctrCmd, user_cmd, sizeof(struct memory_container_cmd));
    oid =  ctrCmd.oid;
    MY_PRINT_MESG(" container get currentNode");
   
    ctrNode = getCurrentContainer();
   
    if(ctrNode == NULL) {
        mutex_unlock(&my_mutex);
        // no container has this task , unlock return
        return 0;
    }

    //MY_PRINT(" release global ", current->pid);
    mutex_unlock(&my_mutex);
    //MY_PRINT_MESG(" unlock local");
    mutex_unlock(&ctrNode->lock);    
    return 0;
}


int memory_container_delete(struct memory_container_cmd __user *user_cmd)
{
    struct container* ctrNode = NULL;
    struct task* temp = NULL;
    struct task* prev = NULL;


    mutex_lock(&my_mutex);
    

    // iterate container list and get the container for current thread 
    ctrNode = getCurrentContainer();

    if(ctrNode == NULL ){
        mutex_unlock(&my_mutex);
        return 0;
    }
    
    if(ctrNode->task_list == NULL ){
        mutex_unlock(&my_mutex);
        return 0;
    }

    if (ctrNode->task_list->next == NULL && ctrNode->task_list->thr == current){
        // only 1 task in container, delete task 
        ctrNode->task_cnt =0;
        kfree(ctrNode->task_list);
        ctrNode->task_list = NULL;
        mutex_unlock(&my_mutex);
        return 0;
    }else {
        // first task to be deleted
        if (current == ctrNode->task_list->thr){
            temp = ctrNode->task_list;
            ctrNode->task_list = ctrNode->task_list->next;
            ctrNode->task_cnt -=1;
            kfree(temp);
        }else{
            temp = ctrNode->task_list;
            prev = temp;
            while(temp != NULL && temp->thr != current){
                prev = temp;
                temp = temp->next;
            }

            if (temp!= NULL && temp->next == NULL && temp->thr==current){
                //printk( "from_delete last node to be deleted  %llu \n",ctrNode->cid);
                // if temp is last node
                ctrNode->task_cnt -=1;
                prev->next = NULL;
                kfree(temp);
            }else if (temp!=NULL && temp->next != NULL && temp->thr == current){
                //printk( "from_delete not the last node to be deleted  %llu \n",ctrNode->cid);
                ctrNode->task_cnt -=1;
                prev->next = temp->next;
                kfree(temp);
            }else{
                // can it even come here ??
                // I dont think so
                mutex_unlock(&my_mutex);
                return 0;
            }
        }
    }
    
    // fail safe mech, if code ever comes here, release the mutex
    mutex_unlock(&my_mutex);
    return 0;
}


int memory_container_create(struct memory_container_cmd __user *user_cmd)
{
    struct container* ctrNode = NULL;
    struct task* tn = NULL;
    struct task* temp = NULL;

    struct memory_container_cmd ctrCmd;
    mutex_lock(&my_mutex);

    memset(&ctrCmd,'\0',sizeof(ctrCmd));

    copy_from_user(&ctrCmd, user_cmd, sizeof(struct memory_container_cmd));
    //MY_PRINT_LLU("create container ",ctrCmd.cid);    
    ctrNode = getContainerFromCid(ctrCmd.cid);

    if (ctrNode != NULL) {
        //MY_PRINT_LLU("container exists, add new task ",ctrNode->cid);    
        //printk(" add %d to %llu \n", current->pid, ctrNode->cid);

        tn = getNewTask();
        //MY_PRINT_MESG("get new task");    
        if ( tn == NULL) {
            mutex_unlock(&my_mutex);
            return 0;
        }    
        
        //MY_PRINT_MESG("post get new taskk");    
            
        ctrNode->task_cnt += 1;
        if (ctrNode->task_list == NULL){
            //MY_PRINT_MESG("add to ctrnode single task");    
            ctrNode->task_list = tn;
            mutex_unlock(&my_mutex);
            return 0;
       
       }

        //MY_PRINT_MESG("loop after get new task");    
        for(temp = ctrNode->task_list; (temp != NULL) && (temp->next != NULL); temp = temp->next)
            ;;
        if(temp->next == NULL)
            temp->next = tn;

    } else{
        ctrNode = getNewContainer(ctrCmd.cid);
        if (ctrNode == NULL) {
            mutex_unlock(&my_mutex);
            return 0;

        }
        
        tn = getNewTask();
        //printk("\n add %d to %llu", current->pid, ctrNode->cid);
        
        if ( tn == NULL) {
            mutex_unlock(&my_mutex);
            return 0;
        }

        ctrNode->task_list = tn;
        
        if(ctr_list == NULL) {
            ctr_list = ctrNode;
        }else{    
            ctrNode->next = ctr_list;
            ctr_list = ctrNode;
        }        
    }

    //MY_PRINT_MESG("unlock and return ");    
    mutex_unlock(&my_mutex);
    return 0;
}


int memory_container_free(struct memory_container_cmd __user *user_cmd)
{
    struct memory_container_cmd ctrCmd ;
    struct container* ctrNode = NULL;
    struct memNode* mm = NULL;
    struct memNode* mm_prev = NULL;
    __u64 oid;
    mutex_lock(&my_mutex);
    memset(&ctrCmd,'\0',sizeof(ctrCmd));
    copy_from_user(&ctrCmd, user_cmd, sizeof(struct memory_container_cmd));
    oid =  ctrCmd.oid;
    
    //MY_PRINT_MESG("global lock takenlock "); 
   
    ctrNode = getCurrentContainer();
     
    if(ctrNode == NULL) {
        // no container has this task , unlock return
        mutex_unlock(&my_mutex);
        return 0;
    }


    //MY_PRINT_LLU("post get container, mcontainer free %llu\n ", ctrNode->cid); 
    //printk("here to delete ctrId %llu objId %llu \n",ctrNode->cid, oid);

    //printObjects(ctrNode);

    mm = ctrNode->mem_list;
  
    if (mm == NULL){
        MY_PRINT_MESG("mm is null\n"); 
        mutex_unlock(&my_mutex);
        return 0;
    }
  
    // 1 element in mmlist
    if (mm->next == NULL){
        if (mm->memory != NULL && mm->oid == oid){
            MY_PRINT_MESG("Single object free now oid match, mm->memoy!=NULL free\n"); 
            if (mm->memory != NULL) {
                kfree(mm->memory);
                mm->memory = NULL;
            }
        }
        kfree(mm);
        ctrNode->mem_list=NULL;
        mutex_unlock(&my_mutex);
        return 0;
    }

    while(mm != NULL){
        if (mm->oid == oid) {
            printk("found oid %llu mm= %p mm_prev= %p \n", oid, mm, mm_prev); 
            break;
        }
        mm_prev = mm;
        mm=mm->next;
    }
   
    //printk("befre deletion mm %p mm_prev %p\n", mm, mm_prev);
    // implement deletion
    if (mm!=NULL && mm->oid == oid){
        if (mm->next == NULL){
            // last node
            //MY_PRINT_MESG("last node\n");
            mm_prev->next = NULL;
        }
        else if ( mm == ctrNode->mem_list){
            // first node
            //MY_PRINT_MESG("first node\n");
            ctrNode->mem_list = mm->next;
            mm->next = NULL;
        }
        else{
            //MY_PRINT_MESG("in middle\n");
            mm_prev->next = mm->next;
            mm->next = NULL;
        // somehwere in mid 
        }
        // free memory
        //MY_PRINT_MESG(" now free");
        kfree(mm->memory);
        kfree(mm);
    }
    //MY_PRINT_LLU("post deletion before returning mcontainer free ", ctrNode->cid); 
    //printObjects(ctrNode);
    mutex_unlock(&my_mutex);
    return 0;
}


/**
 * control function that receive the command in user space and pass arguments to
 * corresponding functions.
 */
int memory_container_ioctl(struct file *filp, unsigned int cmd,
                              unsigned long arg)
{
    switch (cmd)
    {
    case MCONTAINER_IOCTL_CREATE:
        return memory_container_create((void __user *)arg);
    case MCONTAINER_IOCTL_DELETE:
        return memory_container_delete((void __user *)arg);
    case MCONTAINER_IOCTL_LOCK:
        return memory_container_lock((void __user *)arg);
    case MCONTAINER_IOCTL_UNLOCK:
        return memory_container_unlock((void __user *)arg);
    case MCONTAINER_IOCTL_FREE:
        return memory_container_free((void __user *)arg);
    default:
        return -ENOTTY;
    }
}


void _memory_container_exit(void)
{
    struct container* ctrTemp = ctr_list;
    struct memNode* mmTemp = NULL;
    struct task* tskTemp = NULL;

    struct container* q = NULL;
    struct memNode* p = NULL;
    struct task* r = NULL;
    MY_PRINT_MESG("memory cleanup on module de-register");
    while (ctrTemp!=NULL){
        printk("Deletion for Ctr_id %llu \n", ctrTemp->cid);
        
        tskTemp = ctrTemp->task_list;
        while(tskTemp != NULL) {
            r = tskTemp->next;
            //printk("Deleting ctr_id %llu task pid  %d \n", ctrTemp->cid, tskTemp->thr->pid);
            kfree(tskTemp);
            tskTemp=r;
        }
        
        mmTemp=ctrTemp->mem_list;
        printk("Deleting ctr_id %llu mem_obj_ids\n", ctrTemp->cid);
        while(mmTemp!=NULL){
            p = mmTemp->next;
            //printk("Deleting ctr_id %llu mem_obj_id %llu \n", ctrTemp->cid, mmTemp->oid);
            if(mmTemp->memory !=NULL){
                kfree(mmTemp->memory);
            }
            kfree(mmTemp);
            mmTemp=p;
        }
        
        q = ctrTemp->next;
        printk("Deleting Ctr_id %llu \n", ctrTemp->cid);
        kfree(ctrTemp);
        ctrTemp = q;
    }
}
