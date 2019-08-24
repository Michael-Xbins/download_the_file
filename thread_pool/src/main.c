#include "factory.h"

int exit_fds[2];
void sig_exit_func(int signum)
{
	char flag=1;
	write(exit_fds[1],&flag,1); //任何一个线程都可能来响应信号
}

void* download_file(void* p)
{
	pfac pf=(pfac)p;
	pque_t pq=&pf->que;
	pnode_t pcur;
	while(1)
	{
		pthread_mutex_lock(&pq->que_mutex);
		if(0==pq->size)
		{
			pthread_cond_wait(&pf->cond,&pq->que_mutex);
		}
		judge_thread_exit(pq);//切记要放到que_get前面
		que_get(pq,&pcur);
		pthread_mutex_unlock(&pq->que_mutex);
		if(pcur!=NULL)
		{
			tran_file(pcur->new_fd);	
			free(pcur);
		}
	}
}

int main(int argc,char* argv[])
{
	pipe(exit_fds);
	if(fork()) //信号发送给父进程，由父进程通知子进程中的线程池退出
	{
		close(exit_fds[0]);
		signal(SIGUSR1,sig_exit_func);
		pid_t pid=wait(NULL);
		printf("child process pid=%d\n",pid);
		return 0;	//父进程在此结束
	}
	close(exit_fds[1]);
	if(argc!=5)
	{
		printf("./server IP PORT THREAD_NUM CAPACITY");
		return -1;
	}
	factory f;
	int thread_num=atoi(argv[3]);
	int capacity=atoi(argv[4]);
	factory_init(&f,download_file,thread_num,capacity);
	factory_start(&f);

	int sfd;
	tcp_start_listen(&sfd,argv[1],argv[2],capacity);
	int new_fd;
	pque_t pq=&f.que;
	pnode_t pnew;

	int ret;
	int epfd=epoll_create(1);
	struct epoll_event event,evs[2];
	event.events=EPOLLIN;
	event.data.fd=sfd;
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
	check_error(-1,ret,"epoll_ctl");
	event.data.fd=exit_fds[0];
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,exit_fds[0],&event);
	check_error(-1,ret,"epoll_ctl");

	while(1)
	{
		ret=epoll_wait(epfd,evs,2,-1);
		for(int i=0;i<ret;i++)
		{
			if(evs[i].data.fd==sfd)
			{
				new_fd=accept(sfd,NULL,NULL);
				pnew=(pnode_t)calloc(1,sizeof(node_t));
				pnew->new_fd=new_fd;
				pthread_mutex_lock(&pq->que_mutex);
				que_insert(pq,pnew);
				pthread_mutex_unlock(&pq->que_mutex);
				pthread_cond_signal(&f.cond);//用于线程池刚开启还没有接收任务时就要退出，此时队列中还没有放入一个结点，子线程全在睡觉,则此种情形要群体唤醒子线程， 各自去取 退出结点 并 线程退出。
			}
			if(evs[i].data.fd==exit_fds[0])
			{
				close(sfd);
				pnew=(pnode_t)calloc(1,sizeof(node_t));
				pnew->new_fd=-1;  //退出机制
				pthread_mutex_lock(&pq->que_mutex);
				que_insert_exit(pq,pnew);
				pthread_mutex_unlock(&pq->que_mutex);
				pthread_cond_broadcast(&f.cond);//防止在放入退出结点前，队列中一个结点都没有了，子线程此时全在睡觉,则此种情形要群体唤醒。
				for(int j=0;j<thread_num;j++)
				{
					pthread_join(f.pth_id[j],NULL);
				}
				exit(0); //让子进程退出
			}
		}
	}
}
