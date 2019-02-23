#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <asm/delay.h>

#include "ipc.h"

MODULE_LICENSE("GPL");

struct msg_list{
	struct list_head list;
	int index;
	struct data* buf;
	int len;
};

struct msq_list{
	struct list_head list;
	int key;
	struct msg_list msg_head;
	int msg_count;
	int size;
};

struct msq_list my_msq;
int msq_size;
spinlock_t slock;

/* msg function */
static struct msg_list* ipc_msg_create(int index, struct data* new_msg, int len);
static int ipc_msg_delete(struct msg_list* msg);

/* msq function */
static struct msq_list* ipc_msq_get(int msqid);
static struct msq_list* ipc_msq_create(int key);
static int ipc_msq_delete(struct msq_list* msq);

/* ioctl & read & write */ 
static long ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int ipc_write(struct file *file, const char *buf, size_t len, loff_t *lof);
static int ipc_read(struct file *file, char *buf, size_t len, loff_t *lof);
static int ipc_open(struct inode *inode, struct file *file);
static int ipc_release(struct inode *inode, struct file *file);


static struct msg_list* ipc_msg_create(int index, struct data* new_msg, int len){
	int result = -1;
	struct msg_list* msg = NULL;

	msg = (struct msg_list*)kmalloc(sizeof(struct msg_list),GFP_KERNEL);
	msg->index = index;
	msg->len = 0;

	msg->buf = (struct data*)kmalloc(sizeof(struct data),GFP_KERNEL);
	msg->buf->msqid = new_msg->msqid;
	msg->buf->msgflg = new_msg->msgflg;
	msg->buf->msgsz = new_msg->msgsz; 
	msg->buf->msgtyp = new_msg->msgtyp;
	
	msg->buf->msgp = (char*)kmalloc(msg->buf->msgsz, GFP_KERNEL);
	result = copy_from_user(msg->buf->msgp, new_msg->msgp, msg->buf->msgsz);
	if(result<0){
		ipc_msg_delete(msg);
		return NULL;
	}

	return msg;
}


static int ipc_msg_delete(struct msg_list* msg){
	kfree(msg->buf->msgp);
	kfree(msg->buf);
	kfree(msg);
	
	return 0;
}

static struct msq_list* ipc_msq_get(int msqid){
	struct msq_list* tmp = NULL;
	struct msq_list* target = NULL;
	
	list_for_each_entry(tmp, &my_msq.list, list){
		if(tmp->key == msqid){
			target = tmp;
			break;
		}
	}

	return target;
}

static struct msq_list* ipc_msq_create(int key){			// just create msq
	struct msq_list* msq = NULL;
	
	msq = (struct msq_list*)kmalloc(sizeof(struct msq_list), GFP_KERNEL);
	msq->key = key;
	INIT_LIST_HEAD(&msq->msg_head.list);
	msq->msg_count = 0;
	msq->size = 0;

	return msq;		
}

static int ipc_msq_delete(struct msq_list* msq){
	struct list_head* next = NULL;
	struct list_head* pos = NULL;
	struct msg_list* head_msg = &msq->msg_head;
	
	list_for_each_safe(pos,next,&head_msg->list){		// Delete all msgs in the msq
		ipc_msg_delete(list_entry(pos, struct msg_list, list));
	}	
	kfree(msq);
	
	return 0;
}

static long ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	int result = -1;
	int key = *(int*)arg;
	struct msq_list* msq = NULL;
	
	switch(cmd){
		case IOCTL_IPC_CREAT:
			printk("IOCTL CREAT!\n");
			msq = ipc_msq_create(key);
			list_add_tail(&msq->list, &my_msq.list);
			result = msq->key;
			break;
		case IOCTL_IPC_CHECK:
			printk("IOCTL CHECK!\n");
			msq = ipc_msq_get(key);
			if(msq!=NULL)
				result = msq->key;
			else
				result = MY_IPC_NOMSQ;
			break;
		case IOCTL_IPC_CLOSE:
			printk("IOCTL CLOSE!\n");
			msq = ipc_msq_get(key);
			result = msq->key;
			list_del(&msq->list);		// delete list(head) in msq
			ipc_msq_delete(msq);		// delete msg in msq
			break;
		default:
			printk("Wrong Input!\n");
			break;
	}

	return result;
}


static int ipc_open(struct inode *inode, struct file *file){
	printk("ipc open\n");
	return 0;
}

static int ipc_release(struct inode *inode, struct file *file){
	printk("ipc release\n");
	return 0;
}

static int ipc_write(struct file *file, const char *buf, size_t len, loff_t *lof){
	struct msq_list* msq = NULL;
	struct msg_list* new_msg = NULL;
	struct data* user_msg = (struct data*)buf;
	int result  = 0;
	
	spin_lock(&slock);		// start critical section!
	
	msq = ipc_msq_get(user_msg->msqid);
	if(msq==NULL){
		printk("MY_IPC_NOMSQ");
		spin_unlock(&slock);
		return MY_IPC_NOMSQ;
	}
	if((msq->msg_count + 1) > MY_IPC_MAXMSG || (msq->size + len) > MY_IPC_MAXVOL){
		printk("MY_IPC_FULL");
		spin_unlock(&slock);
		return MY_IPC_FULL;
	}

	new_msg = ipc_msg_create(msq->msg_count+1, user_msg, len);		// create msg
	if(new_msg==NULL){
		printk("MY_IPC_ERROR");
		spin_unlock(&slock);
		return MY_IPC_ERROR;
	}
	
	list_add_tail(&new_msg->list, &msq->msg_head.list);
	msq->msg_count++;
	msq->size += len;
	spin_unlock(&slock);
	printk("MSG_SEND_COMPLETE");

	return msq->msg_count;
}

static int ipc_read(struct file *file, char *buf, size_t len, loff_t *lof){
	struct msq_list* msq = NULL;
	struct msg_list* msg = NULL;
	struct data* kernel_msg = NULL;			// target
	struct data* user_msg = (struct data*)buf;	// copy from kernel

	struct list_head* next = NULL;
	struct list_head* pos = NULL;
	struct msg_list* head_msg = NULL;
	struct msg_list* tmp = NULL;

	int result = 0;
	
	spin_lock(&slock);			// critical section start!
	
	/* find msq */
	msq = ipc_msq_get(user_msg->msqid);
	if(msq==NULL){
		spin_unlock(&slock);
		return MY_IPC_NOMSQ;
	}
	if((msq->msg_count - 1) < 0 || msq->size <= 0){
		spin_unlock(&slock);
		return MY_IPC_EMPTY;
	}
	
	/* find msg */
	head_msg = &msq->msg_head;
	list_for_each_safe(pos,next,&head_msg->list){		// find msg in selected msq
		tmp = list_entry(pos, struct msg_list, list);
		if(tmp->buf->msgtyp == user_msg->msgtyp){	// find same msgtype
			msg = tmp;
			kernel_msg = msg->buf;
			break;
		}
	}

	if(kernel_msg==NULL){
		spin_unlock(&slock);
		return MY_IPC_NOMSG;
	}

	/* copy to user_msg*/
	if(len >= kernel_msg->msgsz){
		len = kernel_msg->msgsz;
		result = copy_to_user(user_msg->msgp, kernel_msg->msgp,len);
		if(result<0){
			spin_unlock(&slock);
			return MY_IPC_ERROR;
		}
	}else{
		if((user_msg->msgflg & MY_MSG_NOERROR) != 0){
			result = copy_to_user(user_msg->msgp, kernel_msg->msgp,len);
			if(result<0){
				spin_unlock(&slock);
				return MY_IPC_ERROR;
			}
		}else{
			spin_unlock(&slock);
			return MY_IPC_SHORT;
		}
	}

	/* delete msg from msq */
	list_del(pos);
	ipc_msg_delete(msg);
	msq->msg_count--;
	msq->size -= kernel_msg->msgsz;

	spin_unlock(&slock);

	return len;
}

struct file_operations ipc_fops =
{
	.unlocked_ioctl = ipc_ioctl,
	.open = ipc_open,
	.release = ipc_release,
	.read = ipc_read,
	.write = ipc_write,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ipc_init(void){
	printk("Init Module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ipc_fops);
	cdev_add(cd_cdev, dev_num, 1);
	
	msq_size = 0;
	INIT_LIST_HEAD(&my_msq.list);
	spin_lock_init(&slock);

	return 0;
}

static void __exit ipc_exit(void){
	printk("Exit Module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num,1);
}

module_init(ipc_init);
module_exit(ipc_exit);
