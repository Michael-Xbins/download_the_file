#include "func.h"
int send_fd(int sfd, int fd, int exit_flag)	 //exit_flag：用于通知子进程要退出了
{
	struct msghdr msg;
	bzero(&msg,sizeof(msg));
	char buf[10]="hello";
	struct iovec iov[2];
	iov[0].iov_base=buf; 
	iov[0].iov_len=5;
	iov[1].iov_base=&exit_flag;
	iov[1].iov_len=4;

	msg.msg_iov=iov;
	msg.msg_iovlen=2;
	struct cmsghdr *cmsg;
	int len=CMSG_LEN(sizeof(int));
	cmsg=(struct cmsghdr *)calloc(1,len);
	cmsg->cmsg_len=len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*(int*)CMSG_DATA(cmsg)=fd;
	msg.msg_control=cmsg;
	msg.msg_controllen=len;
	int ret;
	ret=sendmsg(sfd,&msg,0);
	check_error(-1,ret,"sendmsg");
	return 0;
}

int recv_fd(int sfd, int *fd, int *exit_flag)
{
	struct msghdr msg;
	bzero(&msg,sizeof(msg));
	char buf[10]="hello";
	struct iovec iov[2];
	iov[0].iov_base=buf;
	iov[0].iov_len=5;
	iov[1].iov_base=exit_flag;		
	iov[1].iov_len=4;
	msg.msg_iov=iov;
	msg.msg_iovlen=2;
	struct cmsghdr *cmsg;
	int len=CMSG_LEN(sizeof(int));
	cmsg=(struct cmsghdr *)calloc(1,len);
	cmsg->cmsg_len=len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	msg.msg_control=cmsg;
	msg.msg_controllen=len;
	int ret;
	ret=recvmsg(sfd,&msg,0);	   
	//收到传来的msg的内容，其包含iov数组内容（iov_base数据: 用于退出机制）
	check_error(-1,ret,"recvmsg");
	*fd=*(int*)CMSG_DATA(cmsg);
	return 0;
}

