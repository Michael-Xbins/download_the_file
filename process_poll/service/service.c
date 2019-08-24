#include"func.h"
int exit_fds[2];		//定义成全局变量来传参（信号处理函数只能全局变量来传参）

void sig_exit(int signum)	
{
	char exit_flag='a';
	write(exit_fds[1],&exit_flag,1);
}

int main(int argc,char *argv[])
{
	check_args(argc,4);
	pipe(exit_fds); 	//用于服务器退出机制

	int process_num=atoi(argv[3]);   //创建的进程数目
	pData_t p=(pData_t)calloc(process_num,sizeof(Data_t)); 	  //创建指针并申请好空间，来保存创建的各个子进程的信息
	make_child(p,process_num);

	int sfd=socket(AF_INET,SOCK_STREAM,0);
	int ret,i;
	int reuse=1;
	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	check_error(-1,ret,"setsockopt");

	int epfd=epoll_create(1);
	struct epoll_event event,*evs;    //注意：evs空间内还有一个TCP套接字sfd
	evs=(struct epoll_event*)calloc(process_num+1,sizeof(struct epoll_event));
	event.events=EPOLLIN;
	event.data.fd=sfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
	for(i=0;i<process_num;i++)
	{
		event.events=EPOLLIN;
		event.data.fd=p[i].fds;
		ret=epoll_ctl(epfd,EPOLL_CTL_ADD,p[i].fds,&event);
		check_error(-1,ret,"epoll_ctl");
	}
	event.events=EPOLLIN;
	event.data.fd=exit_fds[0];
	epoll_ctl(epfd,EPOLL_CTL_ADD,exit_fds[0],&event);
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	check_error(-1,ret,"bind");
	listen(sfd,process_num+1);

	signal(SIGUSR1,sig_exit);    //用于服务器退出机制
	int read_fd_num,j,new_fd;
	char notbusy_flag=1;
	while(1)
	{
		read_fd_num=epoll_wait(epfd,evs,process_num+1,-1);	 //可读描述符的个数
		for(i=0;i<read_fd_num;i++)
		{
			if(evs[i].data.fd==sfd)
			{
				new_fd=accept(sfd,NULL,NULL);
				for(j=0;j<process_num;j++)
				{
					if(0==p[j].busy_flag)
					{
						send_fd(p[j].fds,new_fd,0);	   //发送描述符给子进程
						p[j].busy_flag=1;
						printf("pid=%d is busy\n",p[j].pid);
						break;
					}
				}
				close(new_fd);   //若子进程都在忙碌时又有新的客户端连接，则新的客户端会直接断开（区别于线程池并不会断开： 因为用了队列机制来保持公平性）
			}

			if(evs[i].data.fd==exit_fds[0])
			{
				close(sfd);
				for(j=0;j<process_num;j++)
				{
					send_fd(p[j].fds,0,-1);
				}
				for(j=0;j<process_num;j++)
				{
					wait(NULL);
				}
				printf("exit success\n");
				return 0;
			}
			//查看是哪个子进程通知父进程自己不忙碌了
			for(j=0;j<process_num;j++)
			{
				if(evs[i].data.fd==p[j].fds)	//某个子进程的fds管道可读
				{
					read(p[j].fds,&notbusy_flag,1);
					p[j].busy_flag=0;
					printf("pid=%d is not busy\n",p[j].pid);
				}
			}
		}
	}
	return 0;
}

