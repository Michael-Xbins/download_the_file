#include "func.h"
int make_child(pData_t p,int process_num)
{
	int i;
	int fds[2];
	pid_t pid;
	for(i=0;i<process_num;i++)
	{
		socketpair(AF_LOCAL,SOCK_STREAM,0,fds); //每个子进程均与父进程之间创建一个特殊管道（可读可写）来传递信息
		pid=fork();
		if(0==pid)
		{
			close(fds[1]);
			child_handle(fds[0]);       //子进程必须在child_handle内进行exit退出
		}
		close(fds[0]);
		p[i].pid=pid;		//记录子进程的pid
		p[i].fds=fds[1];	//记录子进程的特殊管道的对端
		p[i].busy_flag=0;	//忙碌标志，0代表非忙碌，1代表忙碌
	}
	return 0;
}
void child_handle(int fds)
{
	int new_fd;
	int exit_flag=0;
	char notbusy_flag=1;
	while(1)
	{
		recv_fd(fds,&new_fd,&exit_flag);	//等待接收任务
		if(-1==exit_flag)	 //服务器的退出机制
		{
			exit(0);	    //子进程退出
		}
		tran_file(new_fd);
		write(fds,&notbusy_flag,1);    //写管道通知父进程我不忙了
	}
}


