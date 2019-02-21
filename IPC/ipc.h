#ifndef __IPC_HEADER__
#define __IPC_HEADER__

#define IPC_MAXMSG 10
#define IPC_MAXVOL 20

#define IPC_CREAT	0x90
#define IPC_EXCL	0x80
#define IPC_NOWAIT	0x70
#define MSG_NOERROR	0x60
#define IPC_CHECK	0x50
#define IPC_NOMSQ	ox40
#define IPC_NOMSG	0x30
#define IPC_SHORT	0x20

#define IOCTL_START_NUM 0x80
#define IOCTL_CREAT IOCTL_START_NUM+1
#define IOCTL_EXCL IOCTL_START_NUM+2
#define IOCTL_CLOSE IOCTL_START_NUM+3
#define IOCTL_CEHCK IOCTL_START_NUM+4

#define SIMPLE_IOCTL_NUM 'z'
#define IPC_CREAT _IOWR(SIMPLE_IOCTL_NUM, IPC_CREAT, unsigned long *)
#define IPC_EXCL _IOWR(SIMPLE_IOCTL_NUM, IPC_EXCL, unsigned long *)
#define IPC_CLOSE _IOWR(SIMPLE_IOCTL_NUM, IPC_CLOSE, unsigned long *)
#define IPC_CHECK _IOWR(SIMPLE_IOCTL_NUM, IPC_CHECK, unsigned long *)

#define DEV_NAME "ipc_dev"
#define DVE_DIR "/dev/ipc_dev"

struct msg_queue{
	struct list_head list;
	int msqid;
	void *msg[IPC_MAXMSG];
	long type[IPC_MAXMSG];
	int size[IPC_MAXMSG];
	int length;
};

struct data{
	int msqid;		
	void *msgp;
	int msgsz;
	long msgtyp;
	int msgflg;
};

#endif
