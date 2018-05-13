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
#include <map>
#include <sys/time.h>
using namespace std;
#define MAXMSG 200//the stable number of mq_attr.mq_maxmsg. 300 is available.

#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.

#define MAX_TEXT 512  
#define DATANUM 26

map<string,string > resource_R2;
string interest[DATANUM] = {
				"a","b","c","d","e","f","g","h","i","j","k","l","m",
				"n","o","p","q","r","s","t","u","v","w","x","y","z"	
				};
string total_data[DATANUM] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};
void init_resource()
{
	for(int i = 10; i < 18; i++){
		resource_R2[interest[i]] = total_data[i];
	}
}
struct interest_pkt  
{  
    int latency_requirment;  
    char text[MAX_TEXT];  
};  

struct data_msg
{
	int msg_type;
	char text[MAX_TEXT];
};

struct cmd
{
	char text[MAX_TEXT];
	int flag;//1-add 0-delete
};

struct LatencyTestPKT
{
	int route;//(host->)r1->r2->r4 / (host->)r1->r3->r4 标志R2还是R3
	long start_sec,start_usec;
	long duration[3];// host -> r1  r1 -> r2/3   r2/3 -> r4
};
mqd_t mqd_p1top2,mqd_p2top1,mqd_p2top4,mqd_p4top2;
	mqd_t mqd_latencyR1ToR2, mqd_latencyR2ToR4;
	mqd_t mqd_CtrlToR2;
	int mq_return = 0;
	char p1top2[] = "/p1top2";
	char p2top1[] = "/p2top1";
	char p4top2[] = "/p4top2";
	char p2top4[] = "/p2top4";
	char latencyR1ToR2[] = "/latencyR1ToR2";
	char latencyR2ToR4[] = "/latencyR2ToR4";
	char CtrlToR2[] = "/CtrlToR2";
int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
char buffer[2048];
	int running = 1;
	struct cmd pit[10];
	struct LatencyTestPKT *latencyTest;
	struct interest_pkt *data;  
	struct data_msg *datamsg,*r4tor2;
	struct interest_pkt *command;
	struct timeval t_start,t_end; 
	char *found = new char[MAX_TEXT];
	int pit_record_num = 0;
char* find_data(char *index)//index in data out
{
	//struct data_msg datamsg;
	//map later
	string str;

	char * data;
	map<string,string>::iterator iter;
	iter = resource_R2.find(index);
	if(iter != resource_R2.end())
	{
		str = iter->second;
		int len = str.length();
		data = (char *)malloc((len+1)*sizeof(char));
		str.copy(data,len,0);
		return data;
	}
	else return NULL;
}

void print_cs()
{
	cout<<"cs in r2"<<endl;
	map<string,string>::iterator iter;
	for(iter = resource_R2.begin(); iter != resource_R2.end(); iter++)  
		cout<<iter->first<<' '<<iter->second<<endl;  
}


void  *interest_q(void *ptr)
    {
		while(running)
		{
			 if((mq_receive(mqd_p1top2, buffer, 2048, 0)) == -1)
			 {
				cout<<"r1 to r2 receive error"<<strerror(errno)<<"  "<< errno<<endl;
			}
			data = (struct interest_pkt*) buffer;
			cout<<"R2 receive from R1: "<<(*data).text<<endl;  
			datamsg = (struct data_msg*) buffer;
			found =find_data((*datamsg).text);
			//search cs
			//found=NULL;
			if(found!=NULL)
			{
				//already found
				//cout<<"find data pkt::"<<endl;
				strcpy((*datamsg).text,find_data((*data).text));
				//send back data pkt R1 -> host
				if(mq_send(mqd_p2top1, (char *)datamsg, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE); 
				}
				cout<<"R2 send data back to R1: " << (*datamsg).text<<endl;
			}
			
			
			else
			{
				if(mq_send(mqd_p2top4, (char *)data, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				cout<<"R2 send interest to R4: "<< (*data).text << endl;
			}
			
			
			
		}
	//p1top2
		mq_return = mq_close(mqd_p1top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p1top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
		
		//p2top1
		mq_return = mq_close(mqd_p2top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p2top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
		
		//p2top4
		mq_return = mq_close(mqd_p2top4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p2top4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
		
	//	exit(0);
	}
	
	//R4->R2->R1
	void  *data_q(void *ptr)
    {
		while(running)
		{	
			//mq_return = mq_receive(mqd_p4top2, buffer, 2048, 0);
			if((mq_receive(mqd_p4top2, buffer, 2048, 0)) == -1)
			{
				cout<<"r4 to r2 receive error"<<strerror(errno)<<"  "<< errno<<endl;
			}
			r4tor2 = (struct data_msg*) buffer;
			
			//检查是否需要缓存,缓存好之后把cmd记录删掉
				for(int i = 0; i < 10; i++) 
				{
					cout << i <<": "<<pit[i].text<<" - "<<pit[i].flag<<"---"<<(strcmp(pit[i].text, (*r4tor2).text))<<endl;
					if ((strcmp(pit[i].text, (*r4tor2).text) == 32) && (pit[i].flag == 1))
					{
						//缓存 insert 进 cs map
						resource_R2.insert(map<string, string>::value_type (pit[i].text, (*r4tor2).text));
						print_cs();
						pit[i].flag = 0;
						
						break;
					}
				}
			
			//send back data pkt R2 -> R1
			if(mq_send(mqd_p2top1, (char *)r4tor2, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE);  
			}  
			cout<<"R2 send data "<< (*r4tor2).text << "back to R1"<<endl;
		}
		
		//p4top2
		mq_return = mq_close(mqd_p4top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_close");
		mq_return = mq_unlink(p4top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, send0top0, "mq_unlink");
	}

void  *latency_test(void *ptr)
	{
		unsigned long cost_time=0;  
		while(running)
		{
			//receive latency pkt from latencyr1tor2
			mq_return = mq_receive(mqd_latencyR1ToR2, buffer, 2048, 0);
			latencyTest = (struct LatencyTestPKT*) buffer;
			gettimeofday(&t_end, NULL); 
			//calculate time slot 
			cost_time = 10000*(t_end.tv_sec-(*latencyTest).start_sec)+(t_end.tv_usec-(*latencyTest).start_usec);  
			(*latencyTest).duration[1] = cost_time;
			//(*latencyTest).duration[0] *= 1000;
			cout << "R2 receive latency pkt: host <-> R1 duration : " <<(*latencyTest).duration[1] << " send to R"<<(*latencyTest).route <<endl;
			//更新start
			gettimeofday(&t_start, NULL); 
			(*latencyTest).start_sec = t_start.tv_sec;
			(*latencyTest).start_usec = t_start.tv_usec;
			
			if(mq_send(mqd_latencyR2ToR4, (char *)latencyTest, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE);  
			} 
		}
		mq_return = mq_close(mqd_latencyR1ToR2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(latencyR1ToR2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		mq_return = mq_close(mqd_latencyR2ToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(latencyR2ToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
	}

void  *cmd_ctrltor2(void *ptr) 
	{
		//receive cmd from ctrl
		
		while(1)
		{
			mq_return = mq_receive(mqd_CtrlToR2, buffer, 2048, 0);
			command = (struct interest_pkt*) buffer;
			cout<<"command from ctrl: "<< (*command).text << " - "<< (*command).latency_requirment<<endl;
			
			//change pit
			strcpy(pit[pit_record_num%10].text,(*command).text);
			pit[pit_record_num%10].flag = 1;
			pit_record_num++;
			
		}
		
		mq_return = mq_close(mqd_CtrlToR2);
		mq_return = mq_unlink(CtrlToR2);
	}

int main() {
	/*initialization about mqueue*/
	char proname[] = "R2";
	init_resource();

	struct mq_attr attr, attr_ctrl;
	//struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	
	init_resource();
	
	mqd_p1top2 = mq_open(p1top2, flags, PERM, &attr);
	mqd_p2top1 = mq_open(p2top1, flags, PERM, &attr);
	mqd_p2top4 = mq_open(p2top4, flags, PERM, &attr);
	mqd_p4top2 = mq_open(p4top2, flags, PERM, &attr);
	mqd_latencyR1ToR2 = mq_open(latencyR1ToR2, flags, PERM, &attr);
	mqd_latencyR2ToR4 = mq_open(latencyR2ToR4, flags, PERM, &attr);
	mqd_CtrlToR2 = mq_open(CtrlToR2, flags, PERM, &attr);
	if(mqd_p2top4 < 0 )
	{
		cout<<"mqd_p2top4 failed"<<endl;
		cout<<"mqd_p2top4 receive error"<<strerror(errno)<<"  "<< errno<<endl;
		
	}
	
	if(mqd_p4top2 < 0 )
	{
		cout<<"mqd_p4top2 failed"<<endl;
		cout<<"mqd_p4top2 receive error"<<strerror(errno)<<"  "<< errno<<endl;
	}
    pthread_t interest_q_id,data_q_id,latency_test_id,cmd_ctrltor2_id;
    int ret = pthread_create(&interest_q_id, NULL, interest_q, NULL);
    if(ret) {
        cout << "host Create send pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&data_q_id, NULL, data_q, NULL);
    if(ret) {
        cout << "host Create receive pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&latency_test_id, NULL, latency_test, NULL);
    if(ret) {
        cout << "host Create sendtoctrl pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&cmd_ctrltor2_id, NULL, cmd_ctrltor2, NULL);
    if(ret) {
        cout << "host Create sendtoctrl pthread error!" << endl;
        return 1;
    }
    pthread_join(interest_q_id, NULL);
    pthread_join(data_q_id, NULL);
    pthread_join(latency_test_id, NULL);
    pthread_join(cmd_ctrltor2_id, NULL);
  
	exit(0);

}








