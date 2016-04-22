/**
  *@file swclient.c
  *@brief 客户端实现，执行./swclient IP download/upload filename src_path 
  *dst_path
  *@author WuQingwen
  *@history
  *			2016-04-10 WuQingwen created
  */
#include "swmysocket.h"

int main(int argc,const char *argv[])
{
	if( argc != 6 )
	{
		printf("Usage:%s IP upload/download xxx srcpath dstpath\n",argv[0]);
		return 0;
	}

	if( strlen(argv[2]) + 1 > COMMAND_SIZE )
    {
    	printf("请使用upload或者download.\n");
    	return 0;
    }
	//解析命令
	char command_buf[COMMAND_SIZE];
	memset(command_buf,0,sizeof(command_buf));
    strlcpy(command_buf,argv[2],sizeof(command_buf));
	printf("command:%s\n",command_buf);

	if( strncmp(command_buf,"upload",sizeof("upload"))!=0
		&& strncmp(command_buf,"download",sizeof("download"))!=0 )
	{
		printf("Usage:%s upload/download xxx srcpath dstpath\n",argv[0]);
		return 0;
	}
	
	//创建TCP套接字	
	int m_socketfd = socket(AF_INET,SOCK_STREAM,0);
	if( m_socketfd == -1 )
	{
		printf("socket failed!\n");
		return 0;
	}	
	//指定服务器的IP+PORT
	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	int retval = inet_pton(AF_INET,argv[1],&serveraddr.sin_addr);
	if( retval == -1 )
	{
		printf("inet_pton error.\n");
		return 0;
	}
	else if( retval == 0 )
	{
		printf("地址格式错误：例如10.10.1.16.\n");
		return 0;
	}
	serveraddr.sin_port = htons(SERVER_PORT);
	//连接服务端
	if( -1 == connect(m_socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) )
	{
		printf("connect failed!\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
	}
	//解析文件名
	//printf("filename:%s\n",argv[3]);
	
	char filename_buf[FILENAME_SIZE];
	memset(filename_buf,0,FILENAME_SIZE);
	strlcpy(filename_buf,argv[3],sizeof(filename_buf));
	printf("filename:%s\n",filename_buf);

	//发送文件名
	if( sw_send_string(m_socketfd,filename_buf) < 0 )
	{
		printf("write failed!\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
	}

	//发送命令
	if( sw_send_string(m_socketfd,command_buf) < 0 )
	{
		printf("write failed!\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
	}

	char dst_file_path[FILENAME_SIZE];
    memset(dst_file_path,0,FILENAME_SIZE);
    strlcpy(dst_file_path,argv[5],sizeof(dst_file_path));
	strlcat(dst_file_path,filename_buf,sizeof(dst_file_path));	
	//printf("dst:%s\n",dst_file_path);
	
	char src_file_path[FILENAME_SIZE];
    memset(src_file_path,0,FILENAME_SIZE);
	strlcpy(src_file_path,argv[4],sizeof(src_file_path));
	strlcat(src_file_path,filename_buf,sizeof(src_file_path));
	//printf("src:%s\n",src_file_path);

	//下载文件
	if( strncmp(command_buf,"download",sizeof(command_buf))==0 )
	{
		int ret = sw_client_downloadfile(m_socketfd,src_file_path,dst_file_path);
		if( ret == 1 )
			printf("文件:%s下载成功.\n",filename_buf);
		else if( ret == 0 )
		{
			printf("文件:%s下载中断.\n",filename_buf);
		}
		else  //-1
		{
			printf("传输出错.\n");
			unlink(dst_file_path);
			close(m_socketfd);
			m_socketfd = -1;
			return 0;
		}
	}
	//上传文件
	else if( strncmp(command_buf,"upload",sizeof(command_buf))==0 )
	{
		int ret = sw_client_uploadfile(m_socketfd,src_file_path,dst_file_path);
		if( ret == 1 )
			printf("文件:%s上传成功.\n",filename_buf);
		else if( ret == 0 )
			printf("文件:%s上传中断.\n",filename_buf);
		else	//-1
		{
			printf("传输出错.\n");
			close(m_socketfd);
			m_socketfd = -1;
			return 0;
		}
	}
	else  //命令出错
	{
		printf("命令错误.请使用upload或者download.\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
	}
	close(m_socketfd);
	m_socketfd = -1;
	return 0;
}

