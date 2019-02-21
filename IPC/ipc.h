#ifndef __IPC_HEADER__
#define __IPC_HEADER__

#define MY_IPC_MAXMSG 10
#define MY_IPC_MAXVOL 100

#define MY_IPC_CREAT	0x90
#define MY_IPC_EXCL	0x80
#define MY_IPC_NOWAIT	0x70
#define MY_MSG_NOERROR	0x60
#define MY_IPC_CHECK	0x50
#define MY_IPC_NOMSQ	0x40
#define MY_IPC_NOMSG	0x30
#define MY_IPC_SHORT	0x20
#define MY_IPC_FULL	0x10
#define MY_IPC_EMPTY	0xA0
#define MY_IPC_ERROR	0xB0

#define IOCTL_START_NUM 0x80
#define IOCTL_CREAT IOCTL_START_NUM+1
#define IOCTL_CLOSE IOCTL_START_NUM+2
#define IOCTL_CHECK IOCTL_START_NUM+3

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_IPC_CREAT _IOWR(SIMPLE_IOCTL_NUM, IOCTL_CREAT, unsigned long *)
#define IOCTL_IPC_CLOSE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_CLOSE, unsigned long *)
#define IOCTL_IPC_CHECK _IOWR(SIMPLE_IOCTL_NUM, IOCTL_CHECK, unsigned long *)

#define DEV_NAME "ipc_dev"
#define DEV_DIR "/dev/ipc_dev"

struct data{
	int msqid;		
	void *msgp;
	int msgsz;
	long msgtyp;
	int msgflg;
};

#endif
