#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "ipc.h"

#define SUCCESS 0
#define FAIL -1

int msgget(int key, int msgflg);
int msgclose(int msqid);
int msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int msgrcv(int msqid, void *msgp, int msgsz, int msgtyp, int msgflg);

int msgget(int key, int msgflg){
	int dev = open(DEV_DIR,O_RDWR);
	int result = 0;
	
	if(dev<0)
		return FAIL;
	
	result = ioctl(dev, IPC_CHECK, &key);
	if(result == IPC_EXCL)
		result = FAIL;
	else
		result = ioctl(dev, IPC_CREAT, &key);

	close(dev);
	
	return result;
}

int msgclose(int msqid){
	int dev = open(DEV_DIR,O_RDWR);
	int result = -1;
	
	result = ioctl(dev, IPC_CHECK, &key);
	if(result > 0){
		msqid = ioctl(dev, IPC_CLOSE, &key);
		result = 0;
	}
		
	close(dev);
	
	if(result < 0)
		return FAIL;
	else
		return SUCCESS;
}

int msgsnd(int msqid, void *msgp, int msgsz, int msgflg){
	int dev = open(DEV_DIR,O_RDWR);
	int result = -1;
	
	struct data* msgData = NULL;

	if(ioctl(dev, IPC_CHECK, &msqid) < 0){
		close(dev);
		return FAIL;
	}

	msgData = (struct msgData*)malloc(sizeof(struct msgData));
	msgData->msqid = msqid;
	msgData->msgp = msgp;
	msgData->msgsz = msgsz;
	msgData->msgflg = msgflg;

	result = write(dev,msgData,msgsz);
	if(result != 0){
		if((msgflg & IPC_NOWAIT)==0){				// if msgflg=0, no wait
			while(result != 0 && result != IPC_NOMSQ)	// wait..
				result = write(dev,msgData,msgsz);
			free(msgData);
			close(dev);

			if(result == IPC_NOMSQ)
				return FAIL;
			else
				return SUCCESS;
		}
	}	
	
	free(msgData);
	close(dev);

	return SUCCESS;
}

int msgrcv(int msqid, void *msgp, int msgsz, int msgtyp, int msgflg){
	int dev = open(DEV_DIR,O_RDWR);
	int result = -1;
	
	struct data* msgData = NULL;

	if(ioctl(dev, IPC_CHECK, &msqid) < 0){
		close(dev);
		return FAIL;
	}
	
	if((msgflg & MSG_NOERROR) == 0){
		close(dev);
		return FAIL;
	}
		
	msgData = (struct msgData*)malloc(sizeof(struct msgData));
	msgData->msqid = msqid;
	msgData->msgp = msgp;
	msgData->msgsz = msgsz;
	msgData->msgflg = msgflg;
	magData->msgtyp = msgtyp;
	
	result = read(dev,msgData,msgsz);	// result = recv_length
	if(result < 0){		// no data
		if((msgflg & IPC_NOWAIT)==0){
			while(result<0 && result != IPC_NOMSQ && result != IPC_SHORT)
				result = read(dev,msgData, msgsz);
			
			if(result == IPC_NOMSQ || result == IPC_SHORT){
				free(msgData);
				close(dev);
				return FAIL;
			}	
		}else{		
			free(msgData);
			close(dev);
			return FAIL;
		}
	}
	
	free(msgData);
	close(dev);

	return result;
}
