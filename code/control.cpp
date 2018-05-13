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
#include <map>
using namespace std;
#define MAXMSG 200//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  
#define RouteNum 5
#define DATANUM 26
struct interest_pkt  
{  
	int latency_requirment;
    char text[MAX_TEXT];  
};  

struct cmd
{
	int flag;//1-add 0-delete
	char text[MAX_TEXT];
};

string interest[DATANUM] = {
				"a","b","c","d","e","f","g","h","i","j","k","l","m",
				"n","o","p","q","r","s","t","u","v","w","x","y","z"	
				};

struct LatencyTestPKT
{
	int route;//(host->)r1->r2->r4 / (host->)r1->r3->r4 标志R2还是R3
	long start_sec,start_usec;
	long duration[3];// host -> r1  r1 -> r2/3   r2/3 -> r4
};
int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_ctrltohost, mqd_hosttoctrl;
	mqd_t mqd_CtrlToR1, mqd_CtrlToR2, mqd_CtrlToR3, mqd_CtrlToR4, mqd_R4ToCtrl;
	
	int mq_return = 0;
	char ctrltohost[] = "/ctrltohost";
	char hosttoctrl[] = "/hosttoctrl";
	char CtrlToR1[] = "/CtrlToR1";
	char CtrlToR2[] = "/CtrlToR2";
	char CtrlToR3[] = "/CtrlToR3";
	char CtrlToR4[] = "/CtrlToR4";
	char R4ToCtrl[] = "/R4ToCtrl";
	
    struct interest_pkt *data;  
	struct cmd *command;
	struct LatencyTestPKT *latencyTest;
class RouteGragh
{
	private:
		int vertexnum;
		int edgenum;
		int latency[RouteNum][RouteNum];
		//每个路由缓存情况（剩余大小...
		multimap<string,int > resource_route;
		int resource[26][4];
	public:
		//RouteGragh();
		void init()
		{
			//0-host 1-R1 2-R2 3-R3 4-R4
			//init the latency table
			for(int i = 0; i < RouteNum; i++) 
				for(int j = 0; j < RouteNum; j++) 
					latency[i][j] = 0;
			latency[0][1] = latency[1][0] = 20;
			latency[1][2] = latency[2][1] = 30;
			latency[1][3] = latency[3][1] = 30;
			latency[4][3] = latency[3][4] = 40;
			latency[4][2] = latency[2][4] = 40;
			
			//初始化资源位置表
			for(int i = 0; i < 26; i++)
				for(int j = 0; j < 3; j++)
					resource[i][j] = 10;
			for(int i = 0; i < 10; i++)
				resource[i][0] = 1;
			for(int i = 10; i < 18; i++)
				resource[i][0] = 2;
			for(int i = 10; i < 18; i++)
				resource[i][1] = 3;
			for(int i = 18; i < 26; i++)
				resource[i][0] = 4;	
				//R1 0-9 R2 6-15 R3 11-20 R4 17-26
		/*	for(int i = 0; i < 26; i++)
			{
				for(int j = 0; j < 4; j++)
				{
				//	resource[i][j] = 10;
					cout << resource[i][j] << "  ";
				}
				cout<<endl;
			}*/
			
		}
		int getLatency(int beginRoute, int endRoute)
		{
			return latency[beginRoute][endRoute];
		}
		void latencyUpdate(int beginRoute, int endRoute, int latencyNew)
		{
			latency[beginRoute][endRoute] = latencyNew;
		}
		void print_resource()
		{
			for(int i = 0; i < 26; i++)
			{
				for(int j = 0; j < 3; j++)
				{
				//	resource[i][j] = 10;
					cout << resource[i][j] << "  ";
				}
				cout<<endl;
			}
		}
		int findTheRoute(char* interest, int latency_requirment)
		{
			int latency_now = 0;
			int location ;//先假设终点在R1
			int new_location = 0;
			
			//search map (which route) (interest-location)
		/*	multimap<string,int>::iterator iter;
			iter = resource_route.find(interest);
			if(iter != resource_route.end())
				location = iter->second;
			else location = -1;*/
			//init();
			char interest_c = interest[0];
			cout<<"to find "<< interest_c <<endl;
			location = 7;
			for(int i=0;i<3;i++)
			{
				//cout << "resource["<<interest_c-'a'<<"]["<<i<<"]="<<resource[interest_c-'a'][i]<<endl;
					
				if(location > resource[interest_c-'a'][i])
				{
					location = resource[interest_c-'a'][i];
				}
			}
			//print_resource();
			cout<<"found " << *interest << " in R" <<location <<endl;
			new_location = location;
			//if found search the table -> find the route -> get the real-time latency
			if(location == 1)	latency_now += latency[0][1];
			else if (location == 2)		
			{
				latency_now = latency[0][1] + latency[1][2];
				if(latency_now > latency_requirment && latency[0][1] < latency_requirment)
					new_location = 1;
			}
			else if (location == 3)
			{
				latency_now = latency[0][1] + latency[1][3];
				if(latency_now > latency_requirment && latency[0][1] < latency_requirment)
					new_location = 1;
			}
			else if (location == 4)	
			{
				if(latency[1][2] + latency[2][4] < latency[1][3] + latency[3][4])
					latency_now = latency[0][1] + latency[1][2] + latency[2][4];
				else
					latency_now = latency[0][1] + latency[1][3] + latency[3][4];
				
				if(latency_now > latency_requirment)
				{
					if((latency[0][1] + latency[1][3] <= latency_requirment) && (latency[0][1] + latency[1][2] <= latency_requirment))
					{
						srand((unsigned)time(NULL));
						new_location=rand()%2+2;
					}
					else if((latency[0][1] + latency[1][3] <= latency_requirment) && (latency[0][1] + latency[1][2] > latency_requirment))
						new_location = 3;
					else if((latency[0][1] + latency[1][3] > latency_requirment) && (latency[0][1] + latency[1][2] <= latency_requirment))
						new_location = 2;
					else 
					{
						if(latency[0][1] <= latency_requirment)
							new_location = 1;
					}
				}
				
			}
			else
			{
				latency_now = -1;//nofound
				new_location = -1;
				cout << "no found ! "<< endl;
			}	
			//if no found return failed 
			cout<<"found " << *interest << " in R" <<location << "change to R" << new_location<<endl;
			if(new_location == location)
				new_location = 0;//不需要回cmd
			return new_location;
		}
		//更改路由资源表
		void change_cs(char *interest, int newlocation)
		{
			//先找到interest对应的数组下标
			char c = interest[0];
			
			//增加新的location记录
			for(int i = 0; i<4; i++)
			{
				if(resource[c-'a'][i] == 10)	resource[c-'a'][i] = newlocation;
			}
			
		}
		void print_cs()
		{
			for(int i = 0; i < 26; i++)
			{
				for(int j = 0; j < 4; j++)
				{
				//	resource[i][j] = 10;
					cout << resource[i][j] << "  ";
				}
				cout<<endl;
			}
		}
	
};

	int running =1;
	RouteGragh RouteTable;
	char buffer_host[2048], buffer_R4[2048];
	
void *receivefromhost(void *ptr)
	{
	//	cout<<"host receive fork ok"<<endl;
		//char *s = new char[2048];
		while(running)
		{
			//receive data back
			mq_return = mq_receive(mqd_hosttoctrl, buffer_host, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<strerror(errno)<<"  "<< errno<<endl;
	//			return -1;
			}
			data = (struct interest_pkt*) buffer_host;
			cout<<"interest from host ask for:"<<(*data).text
				<<"requiement: "<<(*data).latency_requirment<<endl;
			
			//search
			int find = 0;//new_location
			
			find = RouteTable.findTheRoute((*data).text, (*data).latency_requirment);
			
			if(find!=-1)//找到
			{
				//back the same msg
				mq_return = mq_send(mqd_ctrltohost, (char *)data, 128, 0);
				if(mq_return == -1) {
					cout<<"receive error"<<strerror(errno)<<"  "<< errno<<endl;
	//				return -1;
				}
				(*data).latency_requirment = 1;
				//cmd：给new_location路由缓存指令（异步队列）
				if(find == 1)
				{
					//ctrltor1
					mq_return = mq_send(mqd_CtrlToR1, (char *)data, 128, 0);
					if(mq_return == -1) {
					cout<<"mqd_CtrlToR1 error"<<strerror(errno)<<"  "<< errno<<endl;
	//				return -1;
					}
					cout<<"ctrl send cmd to r1 : "<<(*data).text<<endl;
					//更改ctrl资源表
					
					
				}
				else if (find == 2)
				{
					//ctrltor2
					mq_return = mq_send(mqd_CtrlToR2, (char *)data, 128, 0);
					if(mq_return == -1) {
					cout<<"mqd_CtrlToR2 error"<<strerror(errno)<<"  "<< errno<<endl;
	//				return -1;
					}
					cout<<"ctrl send cmd to r2 : "<<(*data).text<<endl;
					//更改ctrl资源表
					
				}
				else if (find == 3)
				{
					//ctrltor3
					mq_return = mq_send(mqd_CtrlToR3, (char *)data, 128, 0);
					if(mq_return == -1) {
					cout<<"mqd_CtrlToR3 error"<<strerror(errno)<<"  "<< errno<<endl;
	//				return -1;
					}
					cout<<"ctrl send cmd to r3 : "<<(*data).text<<endl;
					//更改ctrl资源表
					
				}
				else
					cout<<"no change for "<<(*data).text<<endl;
				RouteTable.change_cs((*data).text, find);
			}
			else
			{
				//back no found
				strcpy((*data).text,"no");
				mq_return = mq_send(mqd_ctrltohost, (char *)data, 128, 0);
				if(mq_return == -1) {
					cout<<"receive error"<<endl;
//					return -1;
				}
			}
			
		}
		
		mq_return = mq_close(mqd_ctrltohost);
		mq_return = mq_unlink(ctrltohost);
		mq_return = mq_close(mqd_hosttoctrl);
		mq_return = mq_unlink(hosttoctrl);
	//	exit(0); // 结束子进程  
	}
void *receivefromr4(void *ptr)
	{
		mq_return = mq_receive(mqd_R4ToCtrl, buffer_R4, 2048, 0);
		latencyTest = (struct LatencyTestPKT*) buffer_R4;
		
		//update latency table
		RouteTable.latencyUpdate(0,1,(*latencyTest).duration[0]);
		if((*latencyTest).route == 2)
		{
			RouteTable.latencyUpdate(1,2,(*latencyTest).duration[1]);
			RouteTable.latencyUpdate(2,4,(*latencyTest).duration[2]);
		}
		else if((*latencyTest).route == 3)
		{
			RouteTable.latencyUpdate(1,3,(*latencyTest).duration[1]);
			RouteTable.latencyUpdate(3,4,(*latencyTest).duration[2]);
		}
		
		
		
		cout << "ctrl receive latency update msg from r4" << endl;
		mq_return = mq_close(mqd_R4ToCtrl);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(R4ToCtrl);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
	}

int main() {
	/*initialization about mqueue*/
	char proname[] = "control";
	
	//new and init the route table

	struct mq_attr attr, attr_ctrl;
	//struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	

	mqd_ctrltohost = mq_open(ctrltohost, flags, PERM, &attr);
	//check_return(mqd_hosttop1, hosttop1, "mq_open");
	mqd_hosttoctrl = mq_open(hosttoctrl, flags, PERM, &attr);
	mqd_CtrlToR1 = mq_open(CtrlToR1, flags, PERM, &attr);
	mqd_CtrlToR2 = mq_open(CtrlToR2, flags, PERM, &attr);
	mqd_CtrlToR3 = mq_open(CtrlToR3, flags, PERM, &attr);
	mqd_R4ToCtrl= mq_open(R4ToCtrl, flags, PERM, &attr);
	mqd_CtrlToR4= mq_open(CtrlToR4, flags, PERM, &attr);

	pthread_t receivefromhost_id, receivefromr4_id;  
	
    RouteTable.init();
	
	int ret = pthread_create(&receivefromhost_id, NULL, receivefromhost, NULL);
    if(ret) {
        cout << "host Create receivefromhost pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&receivefromr4_id, NULL, receivefromr4, NULL);
    if(ret) {
        cout << "host Create receivefromr4 pthread error!" << endl;
        return 1;
    }
  
	pthread_join(receivefromhost_id, NULL);
    pthread_join(receivefromr4_id, NULL);

//	exit(0);

}








