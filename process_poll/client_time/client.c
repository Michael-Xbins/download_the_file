#include"func.h"

int main(int argc,char *argv[])
{
	check_args(argc,3);
	int sfd;
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ser.sin_port=htons(atoi(argv[2]));
	sfd=socket(AF_INET,SOCK_STREAM,0);
	connect(sfd,(struct sockaddr*)&ser,sizeof(ser));
	int len;
	char buf[1000]={0};

	//接文件名
	recv_n(sfd,(char*)&len,4);
	recv_n(sfd,buf,len);

	//接文件大小
	int fd=open(buf,O_RDWR|O_CREAT,0666);
	check_error(-1,fd,"open");
	off_t file_size;
	double download_size=0;
	recv_n(sfd,(char*)&len,4);
	recv_n(sfd,(char*)&file_size,len);

	//接文件内容:按秒打印下载的百分比
	time_t start,end;
	int ret;
	start=time(NULL);
	while(1)
	{
		bzero(buf,sizeof(buf));
		ret=recv_n(sfd,(char*)&len,4);
		if(ret!=-1&&len>0)
		{
			ret=recv_n(sfd,buf,len);
			if(ret==-1)
			{
				printf("download %5.2lf%s\n",download_size/file_size*100,"%");
				break;
			}
			write(fd,buf,len);
			download_size=download_size+len;
			end=time(NULL);
			if(end-start>=1)
			{
				printf("download %5.2lf%s\r",download_size/file_size*100,"%");
				fflush(stdout);
				start=end;
			}
		}
		else    //len等于0时，即读到了文件结束标志
		{
			printf("download success 100%s\n","%");
			break;
		}
	}
	close(fd);
	close(sfd);
	return 0;
}
