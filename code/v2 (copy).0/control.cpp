/*
 * control
 * thr1:host<->control
 * thr2,3,4,5 : control<->r1,2,3,4
 * 
 * 1.receive from host search( receive interest pkt -> search & find the route -> interest back )
 * 2.give commands to R1-R4
 * 3.receive latency msg from R1-R4(only R4)
 * 4.how to find the route
 * */
 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
using namespace std;
#define MAXMSG 2000//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  

//char DATA[10]

struct interest_pkt  
{  
  //  long int msg_type;  
    char text[MAX_TEXT];  
};  



int main() {
	/*initialization about mqueue*/
	char proname[] = "control";
	

	struct mq_attr attr, attr_ctrl;
	//struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_ctrltohost, mqd_hosttoctrl;
	int mq_return = 0;
	char ctrltohost[] = "/ctrltohost";
	char hosttoctrl[] = "/hosttoctrl";

	
    struct interest_pkt *data;  

	mqd_ctrltohost = mq_open(ctrltohost, flags, PERM, &attr);
	//check_return(mqd_hosttop1, hosttop1, "mq_open");
	mqd_hosttoctrl = mq_open(hosttoctrl, flags, PERM, &attr);

	int running =1;

	//interest pkt to R1
	char buffer[2048];
	
	pid_t receivefromhost;  
	
    
	if((receivefromhost = fork()) == 0)  
	{
		cout<<"host receive fork ok"<<endl;
		
		while(running)
		{
			//receive data back
			mq_return = mq_receive(mqd_hosttoctrl, buffer, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<endl;
				return -1;
			}
			data = (struct interest_pkt*) buffer;
			cout<<"interest from host ask for:"<<(*data).text<<endl;
			
			
			//search
			int find=1;
			if(find==1)
			{
				//back the same msg
				mq_return = mq_send(mqd_ctrltohost, (char *)data, 128, 0);
				if(mq_return == -1) {
					cout<<"receive error"<<endl;
					return -1;
				}
			}
			else
			{
				//back no found
				strcpy((*data).text,"no");
				mq_return = mq_send(mqd_ctrltohost, (char *)data, 128, 0);
				if(mq_return == -1) {
					cout<<"receive error"<<endl;
					return -1;
				}
			}
			
			
		}
		
		mq_return = mq_close(mqd_ctrltohost);
		mq_return = mq_unlink(ctrltohost);
		mq_return = mq_close(mqd_hosttoctrl);
		mq_return = mq_unlink(hosttoctrl);
	//	exit(0); // 结束子进程  
	}
	

	

//	exit(0);

}








