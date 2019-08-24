#include "func.h"
//循环发送
int send_n(int sfd,char* p,int len)
{
	int total=0;
	int ret;
	while(total<len)
	{
		ret=send(sfd,p+total,len-total,0);
		if(-1==ret)				//代表客户端断开：send第一次返回-1
		{
			printf("client is close\n");
			return -1;
		}
		total=total+ret;
	}
	return 0;
}
//循环接收
int recv_n(int sfd,char* p,int len)
{
	int total=0;
	int ret;
	while(total<len)
	{
		ret=recv(sfd,p+total,len-total,0);
		if(0==ret)				// 代表服务器端断开， recv返回0
		{
			return -1;
		}
		total=total+ret;
	}
	return 0;
}

