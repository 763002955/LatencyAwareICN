#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <cstdlib>

using namespace std;
#define MAXMSG 2000//the stable number of mq_attr.mq_maxmsg. 300 is available.

#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.

#define MAX_TEXT 512  

char Index[10][2]={'a','A','b','B','c','C','d','D','e','E','f','F','g','G','h','H','i','I','j','J'};

struct interest_pkt  
{  
  //  long int msg_type;  
    char text[MAX_TEXT];  
};  

struct data_msg
{
	//long int msg_type;
	char text[MAX_TEXT];
};

char* find_data(char *index)//index in data out
{
	
	//map later
	char *str=new char[MAX_TEXT];
    
	for(int i=0;i<10;i++)
	{
		str[0]=Index[i][0];
		if(strcmp(index,str)==0)
		{
	//		cout<<"find index "<< index << "__" << Index[i][1]<<endl;
			//strcpy(datamsg.data,Index[i][1]);
			//return datamsg;
			str[0]=Index[i][1];
			str[1]='\0';
			return str;
		}

	}
	return NULL;
	
}

//set a fib table to determine port

int main() {
	/*initialization about mqueue*/
	char proname[] = "R2";
	

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
	mqd_t mqd_p1top3,mqd_p3top1,mqd_p3top4,mqd_p4top3;
	int mq_return = 0;
	char p1top3[] = "/p1top3";
	char p3top1[] = "/p3top1";
	char p4top3[] = "/p4top3";
	char p3top4[] = "/p3top4";


	mqd_p1top3 = mq_open(p1top3, flags, PERM, &attr);
	mqd_p3top1 = mq_open(p3top1, flags, PERM, &attr);
	mqd_p3top4 = mq_open(p3top4, flags, PERM, &attr);
	mqd_p4top3 = mq_open(p4top3, flags, PERM, &attr);
	
	char buffer[2048];
	int running = 1;
	
	
	struct interest_pkt *data;  
	struct data_msg *datamsg,*r4tor2;
	
    pid_t interest_q,data_q;
    
	char *found = new char[MAX_TEXT];
	
    //R1->R2->R4
    if((interest_q = fork()) == 0)  
    {
		while(running)
		{
			mq_return = mq_receive(mqd_p1top3, buffer, 2048, 0);
			data = (struct interest_pkt*) buffer;
			printf("R1->R2: %s\n",(*data).text);  
			datamsg = (struct data_msg*) buffer;
			//strcpy(found,find_data((*datamsg).text));
			//search cs
			found=NULL;
			if(found!=NULL)
			{
				//already found
				//cout<<"find data pkt::"<< (find_data((*data).text)) <<endl;
				strcpy((*datamsg).text,find_data((*data).text));
				//send back data pkt R1 -> host
				if(mq_send(mqd_p3top1, (char *)datamsg, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE); 
				}
				cout<<"data: R2->R1  " << (*datamsg).text<<endl;
			}
			
			
			else
			{
				if(mq_send(mqd_p3top4, (char *)data, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				cout<<"interest : R2->R4: "<< (*data).text << endl;
			}
			
			
			
		}
	//p1top3
		mq_return = mq_close(mqd_p1top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p1top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
		
		//p3top1
		mq_return = mq_close(mqd_p3top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p3top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
		
		//p3top4
		mq_return = mq_close(mqd_p3top4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p3top4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
		
	//	exit(0);
	}
	
	//R4->R2->R1
    if((data_q = fork()) == 0)  
    {
		while(running)
		{	
			
			mq_return = mq_receive(mqd_p4top3, buffer, 2048, 0);
			r4tor2 = (struct data_msg*) buffer;
			printf("R4->R2: %s\n",(*r4tor2).text);  
			
			//send back data pkt R2 -> R1
			if(mq_send(mqd_p3top1, (char *)r4tor2, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE);  
			}  
			cout<<"send data "<< (*r4tor2).text << "back to R1"<<endl;
		}
		
		//p4top3
		mq_return = mq_close(mqd_p4top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p4top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
	}
	
		
		
	exit(0);

}








