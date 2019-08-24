#include "factory.h"
int factory_init(pfac pf,thread_func_t threadfunc,int thread_num,int capacity)
{
	bzero(pf,sizeof(factory));
	pf->pth_id=calloc(thread_num,sizeof(pthread_t));

	pf->pthread_num=thread_num;
	pf->que.que_capacity=capacity;
	pthread_mutex_init(&pf->que.que_mutex,NULL);
	pf->threadfunc=threadfunc;
	pthread_cond_init(&pf->cond,NULL);
	return 0;
}

int factory_start(pfac pf)
{
	if(!pf->start_flag)
	{
		int i;
		for(i=0;i<pf->pthread_num;i++)
		{
			pthread_create(pf->pth_id+i,NULL,pf->threadfunc,pf);
		}
		pf->start_flag=1;
	}
	return 0;
}

int tcp_start_listen(int *psfd,char *ip,char *port,int backlog)
{
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	int reuse=1;
	int ret;
	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	check_error(-1,ret,"setsockopt");
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(port));
	ser.sin_addr.s_addr=inet_addr(ip);
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(ser));
	check_error(-1,sfd,"bind");
	listen(sfd,backlog);
	*psfd=sfd;
	return 0;
}
