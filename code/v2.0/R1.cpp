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
//fib --- map

struct interest_pkt  
{  
	int latency_requirment;  
    char text[MAX_TEXT];  
};  

struct data_msg
{
	int latency_requirment;  
	char text[MAX_TEXT];
};

char* find_data(char *index)//index in data out
{
	//struct data_msg datamsg;
	//map later
	char *str=new char[MAX_TEXT];
    
	for(int i=0;i<10;i++)
	{
		str[0]=Index[i][0];
		if(strcmp(index,str)==0)
		{
			//cout<<"find index "<< index << "__" << Index[i][1]<<endl;
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
	char proname[] = "R1";
	

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
	mqd_t mqd_hosttop1, mqd_p1tohost, mqd_p1top2, mqd_p1top3, mqd_p2top1,mqd_p3top1;
	int mq_return = 0;
	char hosttop1[] = "/hosttop1";
	char p1tohost[] = "/p1tohost";
	char p1top2[] = "/p1top2";
	char p1top3[] = "/p1top3";
	char p2top1[] = "/p2top1";
	char p3top1[] = "/p3top1";
	
	mqd_hosttop1 = mq_open(hosttop1, flags, PERM, &attr);
	mqd_p1tohost = mq_open(p1tohost, flags, PERM, &attr);
	mqd_p1top2 = mq_open(p1top2, flags, PERM, &attr);
	mqd_p1top3 = mq_open(p1top3, flags, PERM, &attr);
	mqd_p2top1 = mq_open(p2top1, flags, PERM, &attr);
	mqd_p3top1 = mq_open(p3top1, flags, PERM, &attr);
	
	char buffer[2048];
	int running = 1;
	int port = 0;
	
	struct interest_pkt *data;  
	struct data_msg *datamsg;
	struct data_msg *r2tor1,*r3tor1;
	pid_t interest_q,data_q_r2tor1,data_q_r3tor1;
	
	char *found = new char[MAX_TEXT];
	
    if((interest_q = fork()) == 0)  
    {
		while(running)
		{
	 
			//memset(&data.text, 0, 128); 
			mq_return = mq_receive(mqd_hosttop1, buffer, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<endl;
				return -1;
			}
			data = (struct interest_pkt*) buffer;
			datamsg = (struct data_msg*) buffer;
			printf("host -> R1: %s\n",(*datamsg).text);  
			//found = find_data((*data).text);
			//found=NULL;
			strcpy(found,find_data((*data).text));
		//	cout<<"found!"<<(*found)<<endl;
			if(found != NULL)
			{
				cout<<"!null"<<endl;
				//cout<<"find data pkt::"<< (find_data((*data).text)) <<endl;
				strncpy((*datamsg).text,found, MAX_TEXT);
				//(*datamsg).text = "hhh";
				//send back data pkt R1 -> host
				cout<<"!!!!!!!!!!!"<<endl;
				cout<< "send back to host: " << (*datamsg).text << endl;
				
				if(mq_send(mqd_p1tohost, (char *)datamsg, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				cout<<"R1->HOST:"<<(*datamsg).text<<endl;
			}
			else
			{
				//no found
				//fib to choose port
				//search fib find port
				cout<<"to R2 OR R3"<< endl;
				srand((unsigned)time(NULL));
				port=rand()%2+1;
				
				//try port 2
				//port=1;
				
				if(port == 1)//to R2
				{
					//cout<<"R1->R2!"<<endl;
					if(mq_send(mqd_p1top2, (char *)data, 128, 0) == -1)  
					{  
						cout<<"msgsnd back failed"<<endl;
						exit(EXIT_FAILURE);  
					}  
				}
				else if (port == 2)//to R3
				{
					cout<<"R1->R3"<<endl;
					if(mq_send(mqd_p1top3, (char *)data, 128, 0) == -1)  
					{  
						cout<<"msgsnd back failed"<<endl;
						exit(EXIT_FAILURE);  
					}  
				}
				else 
					cout << "error port " << endl;
				
				cout << "send interest " << (*data).text << "to port "<< port <<endl;
			}

		}
	
		//hosttop1
		mq_return = mq_close(mqd_hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		//p1tohost
		mq_return = mq_close(mqd_p1tohost);
		mq_return = mq_unlink(p1tohost);
		
		//p1top2
		mq_return = mq_close(mqd_p1top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p1top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		//p1top3
		mq_return = mq_close(mqd_p1top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p1top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		//exit(0);
	}
	
	if((data_q_r2tor1 = fork()) == 0)
	{
		
		//use 1 thread to receive R2->R1 pkt
			while(running)
			{
				//R2->R1
				mq_return = mq_receive(mqd_p2top1, buffer, 2048, 0);
				r2tor1 = (struct data_msg*) buffer;
				printf("data : R2 back ->R1: %s\n",(*r2tor1).text);  
				
				//......
				if(mq_send(mqd_p1tohost, (char *)r2tor1, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				
				
			}
		
		//p2top1
		mq_return = mq_close(mqd_p2top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p2top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		//	exit(0);
	}	
	
	if((data_q_r3tor1 = fork()) == 0)
	{
		
		//use 1 thread to receive R3->R1 pkt
			while(running)
			{
				//R3->R1
				mq_return = mq_receive(mqd_p3top1, buffer, 2048, 0);
				r3tor1 = (struct data_msg*) buffer;
				printf("data :  R3 back->R1: %s\n",(*r3tor1).text); 
				//..... 
				if(mq_send(mqd_p3top1, (char *)r3tor1, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				} 
				
			}
		
		//p3top1
		mq_return = mq_close(mqd_p3top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p3top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		//	exit(0);
	}	
	exit(0);

}







