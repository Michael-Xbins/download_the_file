#include "factory.h"

void sigfunc(int signum)
{
	printf("%d is coming\n",signum);
}

int tran_file(int new_fd)
{
	train t;
	signal(SIGPIPE,sigfunc);

	//先发文件名
	t.len=strlen(FILENAME);
	strcpy(t.buf,FILENAME);
	int ret;
	ret=send_n(new_fd,(char*)&t,4+t.len);
	if(-1==ret)			    //若客户端在下载的过程中突然断开了，则服务器端 tran_file.c的 send第一次返回-1时就要做判断，下同。
	{
		goto end;
	}

	//发文件大小
	int fd=open(FILENAME,O_RDONLY);
	check_error(-1,fd,"open");
	struct stat buf;
	fstat(fd,&buf);
	t.len=sizeof(buf.st_size);   // t.len中存的是8个字节（off_t是8个字节）注意：此处要传的是8，而不是长整型数: 因为文件大小 即是要发送的 文件内容。
	memcpy(t.buf,&buf.st_size,sizeof(off_t));//传长整型数(文件大小)到buf中
	ret=send_n(new_fd,(char*)&t,4+t.len);
	if(-1==ret)
	{
		goto end;
	}

	//发文件内容
	while((t.len=read(fd,t.buf,sizeof(t.buf)))>0)
		//注意： t.len一定不能用strlen来算
	{
		ret=send_n(new_fd,(char*)&t,4+t.len); 
		if(-1==ret)
		{
			goto end;
		}
	}

	//发送结束标志
	ret=send_n(new_fd,(char*)&t,4+t.len);	 //此时刚好t.len中存的就是0
	if(-1==ret)
	{
		goto end;
	}
end:
	close(new_fd);
	return 0;
}

