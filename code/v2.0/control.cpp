/*
 * control
 * thr1:host<->control
 * thr2,3,4,5 : control<->r1,2,3,4
 * 
 * 1.receive from host search( receive interest pkt -> search & find the route -> interest back )
 * 2.give commands to R1-R4
 * 3.receive latency msg from R1-R4(only R4)
 * 4.how to find the route dijkstra
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
#define RouteNum 5

struct interest_pkt  
{  
	int latency_requirment;
    char text[MAX_TEXT];  
};  

class RouteGragh
{
	private:
		int vertexnum;
		int edgenum;
		int latency[RouteNum][RouteNum];
	public:
		RouteGragh()
		{
			//0-host 1-R1 2-R2 3-R3 4-R4
			//init
			for(int i = 0; i < RouteNum; i++) 
				for(int j = 0; j < RouteNum; j++) 
					latency[i][j] = 0;
			latency[0][1] = latency[1][0] = 2;
			latency[1][2] = latency[2][1] = 2;
			latency[1][3] = latency[3][1] = 2;
			latency[4][3] = latency[3][4] = 2;
			latency[4][2] = latency[2][4] = 2;
			
		}
		int getLatency(int beginRoute, int endRoute)
		{
			return latency[beginRoute][endRoute];
		}
		void latencyUpdate(int beginRoute, int endRoute, int latencyNew)
		{
			latency[beginRoute][endRoute] = latencyNew;
		}
		int findTheRoute(char* interest, int latency_requirment)
		{
			int latency_now = 0;
			//search map (which route) (interest-location)
			int location = 1;//先假设终点在R1
			
			//if found search the table -> find the route -> get the real-time latency
			if(location == 1)	latency_now += latency[0][1];
			else if (location == 2)		latency_now = latency[0][1] + latency[1][2];
			else if (location == 3)		latency_now = latency[0][1] + latency[1][3];
			else if (location == 4)	
			{
				if(latency[1][2] + latency[2][4] < latency[1][3] + latency[3][4])
					latency_now = latency[0][1] + latency[1][2] + latency[2][4];
				else
					latency_now = latency[0][1] + latency[1][3] + latency[3][4];
			}
			else
				latency_now = -1;//nofound
			//if no found return failed 
			return 1;//return???????
		}
		
};

int main() {
	/*initialization about mqueue*/
	char proname[] = "control";
	
	//new and init the route table
	RouteGragh RouteTable;
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
			cout<<"interest from host ask for:"<<(*data).text
				<<"requiement: "<<(*data).latency_requirment<<endl;
			
			//search
			int find = 0;
			
			find = RouteTable.findTheRoute((*data).text, (*data).latency_requirment);
			
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








