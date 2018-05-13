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

#define MAXMSG 2000//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  

int main() {
	/*initialization about mqueue*/
	char proname[] = "clear";
	
	struct mq_attr attr, attr_ctrl;
	//struct mq_attr q_attr;
	attr.mq_maxmsg = 1000;
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	int flags = O_CREAT | O_RDWR;
	
	mqd_t mqd_ctrltohost, mqd_hosttoctrl ,mqd_CtrlToR1,mqd_CtrlToR3,mqd_R4ToCtrl,mqd_CtrlToR4,mqd_p1top2,mqd_p1top3,mqd_p2top1 ;
	mqd_t mqd_p3top1,mqd_latencyR1ToR2,mqd_latencyR1ToR3,mqd_latencyHostToR1,mqd_latencyR3ToR4,mqd_latencyR2ToR4,mqd_p3top4;
	mqd_t mqd_p4top3,mqd_p2top4,mqd_p4top2,mqd_CtrlToR2 ;
	mqd_t mqd_test21;
	int mq_return = 0;
	char test21[] = "/test21";
	char ctrltohost[] = "/ctrltohost";
	char hosttoctrl[] = "/hosttoctrl";
	char CtrlToR1[] = "/CtrlToR1";
	char CtrlToR2[] = "/CtrlToR2";
	char CtrlToR3[] = "/CtrlToR3";
	char CtrlToR4[] = "/CtrlToR4";
	char R4ToCtrl[] = "/R4ToCtrl";
	char hosttop1[] = "/hosttop1";
	char p1tohost[] = "/p1tohost";
	char p1top3[] = "/p1top3";
	char p3top1[] = "/p3top1";
	char latencyR1ToR2[] = "/latencyR1ToR2";
	char latencyR1ToR3[] = "/latencyR1ToR3";
	char latencyHostToR1[] = "/latencyHostToR1";
	char p1top2[] = "/p1top2";
	char p4top2[] = "/p4top2";
	char p2top4[] = "/p2top4";
	char latencyR2ToR4[] = "/latencyR2ToR4";
	char p2top1[] = "/p2top1";
	char p4top3[] = "/p4top3";
	char p3top4[] = "/p3top4";
	
	char latencyR3ToR4[] = "/latencyR3ToR4";
	mqd_ctrltohost = mq_open(ctrltohost, flags, PERM, &attr);
	if(mqd_ctrltohost < 0 )
		cout<<"mqd_ctrltohost open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_hosttoctrl = mq_open(hosttoctrl, flags, PERM, &attr);
	if(mqd_hosttoctrl < 0 )
		cout<<"mqd_hosttoctrl open error"<<strerror(errno)<<"  "<< errno<<endl;

	mqd_CtrlToR1 = mq_open(CtrlToR1, flags, PERM, &attr);
	if(mqd_CtrlToR1 < 0 )
		cout<<"mqd_CtrlToR1 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_CtrlToR2 = mq_open(CtrlToR2, flags, PERM, &attr);
	if(mqd_CtrlToR2 < 0 )
		cout<<"mqd_CtrlToR2 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_CtrlToR3 = mq_open(CtrlToR3, flags, PERM, &attr);
	if(mqd_CtrlToR3 < 0 )
		cout<<"mqd_CtrlToR3 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_R4ToCtrl= mq_open(R4ToCtrl, flags, PERM, &attr);
	if(mqd_R4ToCtrl < 0 )
		cout<<"mqd_R4ToCtrl open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_CtrlToR4= mq_open(CtrlToR4, flags, PERM, &attr);
	if(mqd_CtrlToR4 < 0 )
		cout<<"mqd_CtrlToR4 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mqd_p1top2 = mq_open(p1top2, flags, PERM, &attr);
	if(mqd_p1top2 < 0 )
		cout<<"mqd_p1top2 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_p1top3 = mq_open(p1top3, flags, PERM, &attr);
	if(mqd_p1top3 < 0 )
		cout<<"mqd_p1top3 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_p2top1 = mq_open(p2top1, flags, PERM, &attr);
	if(mqd_p2top1 < 0 )
		cout<<"mqd_p2top1 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_p3top1 = mq_open(p3top1, flags, PERM, &attr);
	if(mqd_p3top1 < 0 )
		cout<<"mqd_p3top1 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mqd_latencyR1ToR2 = mq_open(latencyR1ToR2, flags, PERM, &attr);
	if(mqd_latencyR1ToR2 < 0 )
		cout<<"mqd_latencyR1ToR2 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_latencyR1ToR3 = mq_open(latencyR1ToR3, flags, PERM, &attr);
	if(mqd_latencyR1ToR3 < 0 )
		cout<<"mqd_latencyR1ToR3 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_latencyHostToR1 = mq_open(latencyHostToR1, flags, PERM, &attr);
	if(mqd_latencyHostToR1 < 0 )
		cout<<"mqd_latencyHostToR1 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_latencyR3ToR4 = mq_open(latencyR3ToR4, flags, PERM, &attr);
	if(mqd_latencyR3ToR4 < 0 )
		cout<<"mqd_latencyR3ToR4 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_latencyR2ToR4 = mq_open(latencyR2ToR4, flags, PERM, &attr);
	if(mqd_latencyR2ToR4 < 0 )
		cout<<"mqd_latencyR2ToR4 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mqd_p3top4 = mq_open(p3top4, flags, PERM, &attr);
	if(mqd_p3top4 < 0 )
		cout<<"mqd_p3top4 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_p4top3 = mq_open(p4top3, flags, PERM, &attr);
	if(mqd_p4top3 < 0 )
		cout<<"mqd_p4top3 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_p2top4 = mq_open(p2top4, flags, PERM, &attr);
	if(mqd_CtrlToR2 < 0 )
		cout<<"mqd_CtrlToR2 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_p4top2 = mq_open(p4top2, flags, PERM, &attr);
	if(mqd_p4top2 < 0 )
		cout<<"mqd_p4top2 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mqd_test21= mq_open(test21, flags, PERM, &attr);
	if(mqd_test21 < 0 )
		cout<<"mqd_test21 open error"<<strerror(errno)<<"  "<< errno<<endl;
		
	//关闭
	mq_return = mq_close(mqd_ctrltohost);
	if(mq_return < 0)	cout<<"mqd_ctrltohost close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(ctrltohost);
	if(mq_return < 0)	cout<<"mqd_ctrltohost unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	mq_return = mq_close(mqd_hosttoctrl);
	if(mq_return < 0)	cout<<"mqd_hosttoctrl close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(hosttoctrl);
	if(mq_return < 0)	cout<<"mqd_hosttoctrl unlink error"<<strerror(errno)<<"  "<< errno<<endl;

	mq_return = mq_close(mqd_CtrlToR2);
	if(mq_return < 0)	cout<<"mqd_CtrlToR2 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(CtrlToR2);
	if(mq_return < 0)	cout<<"mqd_CtrlToR2 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	mq_return = mq_close(mqd_CtrlToR1);
	if(mq_return < 0)	cout<<"mqd_CtrlToR1 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(CtrlToR1);
	if(mq_return < 0)	cout<<"mqd_CtrlToR1 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	mq_return = mq_close(mqd_CtrlToR3);
	if(mq_return < 0)	cout<<"mqd_CtrlToR3 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(CtrlToR3);
	if(mq_return < 0)	cout<<"mqd_CtrlToR3 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	mq_return = mq_close(mqd_R4ToCtrl);
	if(mq_return < 0)	cout<<"mqd_R4ToCtrl close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(R4ToCtrl);
	if(mq_return < 0)	cout<<"mqd_R4ToCtrl unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	mq_return = mq_close(mqd_CtrlToR4);
	if(mq_return < 0)	cout<<"mqd_CtrlToR4 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(CtrlToR4);
	if(mq_return < 0)	cout<<"mqd_CtrlToR4 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	mq_return = mq_close(mqd_p1top2);
	if(mq_return < 0)	cout<<"mqd_p1top2 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p1top2);
	if(mq_return < 0)	cout<<"mqd_p1top2 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	
	mq_return = mq_close(mqd_p1top3);
	if(mq_return < 0)	cout<<"mqd_p1top3 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p1top3);
	if(mq_return < 0)	cout<<"mqd_p1top3 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	
	mq_return = mq_close(mqd_p2top1);
	if(mq_return < 0)	cout<<"mqd_p2top1 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p2top1);
	if(mq_return < 0)	cout<<"mqd_p2top1 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_p3top1);
	if(mq_return < 0)	cout<<"mqd_p3top1 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p3top1);
	if(mq_return < 0)	cout<<"mqd_p3top1 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_latencyR1ToR2);
	if(mq_return < 0)	cout<<"mqd_latencyR1ToR2 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(latencyR1ToR2);
	if(mq_return < 0)	cout<<"mqd_latencyR1ToR2 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		

	mq_return = mq_close(mqd_latencyR1ToR3);
	if(mq_return < 0)	cout<<"mqd_latencyR1ToR3 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(latencyR1ToR3);
	if(mq_return < 0)	cout<<"mqd_latencyR1ToR3 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_latencyHostToR1);
	if(mq_return < 0)	cout<<"mqd_latencyHostToR1 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(latencyHostToR1);
	if(mq_return < 0)	cout<<"mqd_latencyHostToR1 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_latencyR3ToR4);
	if(mq_return < 0)	cout<<"mqd_latencyR3ToR4 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(latencyR3ToR4);
	if(mq_return < 0)	cout<<"mqd_latencyR3ToR4 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_latencyR2ToR4);
	if(mq_return < 0)	cout<<"mqd_latencyR2ToR4 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(latencyR2ToR4);
	if(mq_return < 0)	cout<<"mqd_latencyR2ToR4 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	
	mq_return = mq_close(mqd_p3top4);
	if(mq_return < 0)	cout<<"mqd_p3top4 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p3top4);
	if(mq_return < 0)	cout<<"mqd_p3top4 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_p4top3);
	if(mq_return < 0)	cout<<"mqd_p4top3 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p4top3);
	if(mq_return < 0)	cout<<"mqd_p4top3 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		

	mq_return = mq_close(mqd_p2top4);
	if(mq_return < 0)	cout<<"mqd_p2top4 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p2top4);
	if(mq_return < 0)	cout<<"mqd_p2top4 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
		
	
	mq_return = mq_close(mqd_p4top2);
	if(mq_return < 0)	cout<<"mqd_p4top2 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(p4top2);
	if(mq_return < 0)	cout<<"mqd_p4top2 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
	
	mq_return = mq_close(mqd_test21);
	if(mq_return < 0)	cout<<"mqd_test21 close error"<<strerror(errno)<<"  "<< errno<<endl;
	mq_return = mq_unlink(test21);
	if(mq_return < 0)	cout<<"mqd_test21 unlink error"<<strerror(errno)<<"  "<< errno<<endl;
	
}








