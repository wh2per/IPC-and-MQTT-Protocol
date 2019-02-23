#include <stdio.h>
#include <stdlib.h>
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
	
	result = ioctl(dev, IOCTL_IPC_CHECK, &key);
	if(result != MY_IPC_NOMSQ){		// exist
		if(msgflg == MY_IPC_EXCL)
			result = FAIL;
	}else							// not exist
		result = ioctl(dev, IOCTL_IPC_CREAT, &key);

	close(dev);
	return result;
}

int msgclose(int msqid){
	int dev = open(DEV_DIR,O_RDWR);
	int result = -1;
	
	result = ioctl(dev, IOCTL_IPC_CHECK, &msqid);
	if(result != MY_IPC_NOMSQ)
		result = ioctl(dev, IOCTL_IPC_CLOSE, &msqid);
	
	close(dev);
	
	if(result == MY_IPC_NOMSQ)
		return FAIL;
	else
		return result;
}

int msgsnd(int msqid, void *msgp, int msgsz, int msgflg){
	int dev = open(DEV_DIR,O_RDWR);
	int result = -1;
	
	struct data* msgData = NULL;

	if(ioctl(dev, IOCTL_IPC_CHECK, &msqid) < 0){
		close(dev);
		return FAIL;
	}
	printf("start making msg...\n");
	msgData = (struct msgData*)malloc(sizeof(struct data));
	msgData->msqid = msqid;
	msgData->msgp = msgp;
	msgData->msgsz = msgsz;
	msgData->msgflg = msgflg;
	msgData->msgtyp = 1;
	printf("end making msg...\n");

	result = write(dev,msgData,msgsz);
	if(result == MY_IPC_FULL || result == MY_IPC_ERROR || result == MY_IPC_NOMSQ){
		if(msgflg != MY_IPC_NOWAIT){			// if msgflg!=112, wait
			printf("wait...\n");
			while(result == MY_IPC_FULL && result !=MY_IPC_NOMSQ && result != MY_IPC_ERROR)	// wait..
				result = write(dev,msgData,msgsz);
			
			free(msgData);
			close(dev);
			if(result==MY_IPC_ERROR)		// no msg
				return -3;
			else if(result==MY_IPC_NOMSQ)
				return -2;
			else
				return result;
		}else{
			printf("not wait...\n");
			free(msgData);
			close(dev);
			if(result==MY_IPC_FULL)
				return -4;
			else if(result == MY_IPC_ERROR)
				return -3;
			else if(result == MY_IPC_NOMSQ)
				return -2;
			else
				return result;
		}
	}
	
	free(msgData);
	close(dev);

	return result;
}

int msgrcv(int msqid, void *msgp, int msgsz, int msgtyp, int msgflg){
	int dev = open(DEV_DIR,O_RDWR);
	int result = -1;
	
	struct data* msgData = NULL;

	if(ioctl(dev, IOCTL_IPC_CHECK, &msqid) < 0){
		close(dev);
		return -5;
	}
		
	msgData = (struct msgData*)malloc(sizeof(struct data));
	msgData->msqid = msqid;
	msgData->msgp = msgp;
	msgData->msgsz = msgsz;
	msgData->msgflg = msgflg;
	msgData->msgtyp = msgtyp;
	
	result = read(dev,msgData,msgsz);	// result = recv_length
	if(result == MY_IPC_NOMSQ || result == MY_IPC_SHORT || result == MY_IPC_EMPTY || result == MY_IPC_NOMSG || result == MY_IPC_ERROR){		// no data
		if(msgflg != MY_IPC_NOWAIT){	// if msgflg!=112, no wait
			printf("wait...\n");
			while((result == MY_IPC_SHORT || result == MY_IPC_EMPTY || result == MY_IPC_NOMSG) && result != MY_IPC_ERROR && result != MY_IPC_NOMSQ)	// wait..
				result = read(dev,msgData, msgsz);
				
				free(msgData);
				close(dev);
				if(result == MY_IPC_NOMSQ)
					return -5;
				else if(result == MY_IPC_ERROR)
					return -2;
				else
					return result;
			
		}else{	
			printf("not wait...\n");	
			free(msgData);
			close(dev);
			if(result == MY_IPC_NOMSG)
				return -6;
			else if(result == MY_IPC_NOMSQ)
				return -5;
			else if(result == MY_IPC_SHORT)
				return -4;
			else if(result == MY_IPC_EMPTY)
				return -3;
			else if(result == MY_IPC_ERROR)
				return -2;
			else
				return result;
		}
	}
	
	free(msgData);
	close(dev);

	return result;
}

int main(void){
	printf("IPC Start!\n");
	int menu=0;

	while(menu<5){
		printf("\n\n1. create_msq 2. close_msq  3. msg_snd  4. msg_rcv\n");
		printf("select menu : ");
		scanf("%d",&menu);
		
		switch(menu){
			case 1:		// msg get
			{
				int msqid=0;
				int result=0;
				printf("create msq! -> ");
				printf("msqid : ");
				scanf("%d",&msqid);
				result = msgget(msqid,MY_IPC_EXCL);
				if(result!=-1)
					printf("msq create Complete : %d\n",result);
				else
					printf("this msqid is already exist!\n");
				break;
			}
			case 2:		// msg close
			{
				int msqid=0;
				int result=0;
				printf("close msq! -> ");
				printf("msqid : ");
				scanf("%d",&msqid);
				result = msgclose(msqid);
				if(result!=-1)
					printf("msq close Complete : %d\n",result);
				else
					printf("this msqid is not exist!\n");
				break;
			}
			case 3:		// msg snd
			{
				int msqid=0;
				int result=0;
				int msgflg=0;
				char* msgp = malloc(sizeof(char)*10);
				printf("send msg! -> ");
				printf("msqid : ");
				scanf("%d",&msqid);
				printf("msg : ");
				scanf("%s",msgp);
				printf("msgflg : ");
				scanf("%d",&msgflg);
				result = msgsnd(msqid, (void*)msgp, 10, msgflg);
				if(result==-1)
					printf("msg send Fail!\n");
				else if(result==-2)
					printf("msg send Fail! : MY_IPC_NOMSQ\n");
				else if(result==-3)
					printf("msg send Fail! : MY_IPC_ERROR\n");
				else if(result==-4)
					printf("msg send Fail! : MY_IPC_FULL\n");
				else
					printf("msg send Complete : %d\n",result);
				break;
			}
			case 4:		// msg rcv
			{
				int msqid=0;
				int result=0;
				int msgtyp=0;
				int msgflg=0;
				char* msgp = malloc(sizeof(char)*10);
				printf("recv msg! -> ");
				printf("msqid : ");
				scanf("%d",&msqid);
				printf("msgtyp : ");
				scanf("%d",&msgtyp);
				printf("msgflg : ");
				scanf("%d",&msgflg);
				result = msgrcv(msqid, (void*)msgp,10,msgtyp,msgflg);
				if(result==-1)
					printf("msg receive Fail!\n");
				else if(result==-2)
					printf("msg receive Fail! : MY_IPC_ERROR\n");
				else if(result==-3)
					printf("msg receive Fail! : MY_IPC_EMPTY\n");
				else if(result==-4)
					printf("msg receive Fail! : MY_IPC_SHORT\n");
				else if(result==-5)
					printf("msg receive Fail! : MY_IPC_NOMSQ\n");
				else if(result==-6)
					printf("msg receive Fail! : MY_IPC_NOMSG\n");
				else{
					printf("msg receive Complete : %d\n",result);
					printf("msg : %s\n",msgp);
				}
				break;
			}
			default:
				break;
		}
	}
	
}
