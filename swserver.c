/**
  *@file swserver.c
  *@brief 服务端的实现，执行./swserver
  *@author WuQingwen
  *@history 
  *			2016-04-10 WuQingwen created
  */

#include "swmysocket.h"

int main(int argc,char *argv[])
{
	if( argc != 1)
	{
		printf("请执行./swserver\n");
		return 0;
	}
	//创建TCP套接字
	int m_socketfd = socket(AF_INET,SOCK_STREAM,0);
	if(m_socketfd == -1)
	{
		printf("socket failed!\n");
		return 0;
	}
	//绑定 server IP + PORT
	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);
	if(-1 == bind(m_socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)))
	{	
		printf("bind failed!\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
	}
	//解除端口占用
	int on= 1;
 	if( -1 == setsockopt(m_socketfd, SOL_SOCKET,SO_REUSEADDR, 
 							(const char*)&on, sizeof(on)) )
 	{
		printf("setsockopt failed.\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
 	}	
 	//将套接字设置为监听状态，并且设置同时接受连接个数的最大值3+4
	if( -1 == listen(m_socketfd,3) )
	{
		printf("listen failed!\n");
		close(m_socketfd);
		m_socketfd = -1;
		return 0;
	}

	struct sockaddr_in clientaddr; //对方客户端地址结构
	int clientaddr_len = sizeof(clientaddr);
	int  m_connectfd = -1;

	char peer_addr[16];
	char filename_buf[FILENAME_SIZE];
	char command_buf[9];

	while(1)
	{
		memset(&clientaddr,0,clientaddr_len);
		//等待客户端连接
		m_connectfd = accept(m_socketfd,(struct sockaddr*)&clientaddr,&clientaddr_len);
		if( m_connectfd == -1 )
		{
			printf("accept failed!\n");
			continue;
		}
		memset(peer_addr,0,sizeof(peer_addr));
		printf("new connection[%s:%hu]\n",
			inet_ntop(AF_INET,&clientaddr.sin_addr,peer_addr,clientaddr_len),
			ntohs(clientaddr.sin_port));
		
		//接收要下载的文件名
		int filename_len = 0;
		if( read(m_connectfd,&filename_len,sizeof(int))<0 )
		{
			printf("read filename len  failed!\n");
	        close(m_connectfd); 
			m_connectfd = -1;
			continue;
		}
		//printf("filename_len=%d\n",filename_len);
		memset(filename_buf,0,FILENAME_SIZE);
		if( read(m_connectfd,filename_buf,filename_len)<0 )
		{
			printf("read filename failed!\n");
	        close(m_connectfd); 
			m_connectfd = -1;
			continue;
		}
		printf("filename:%s\n",filename_buf);
		
		//接收命令
		int command_len =0;
		if( read(m_connectfd,&command_len,sizeof(int))<0 )
		{
			printf("read filename failed!\n");
	        close(m_connectfd); 
			m_connectfd = -1;
			continue;
		}
		//printf("command_len=%d\n",command_len);
		memset(command_buf,0,sizeof(command_buf));
		if( read(m_connectfd,command_buf,command_len)<0 )
		{
			printf("read command fail.\n");
	        close(m_connectfd); 
			m_connectfd = -1;
			continue;
		}
		printf("command:%s\n",command_buf);	
		
		//下载文件
		if( strncmp(command_buf,"download",sizeof("download"))==0 )
		{
			int transfer_state = sw_server_downloadfile(m_connectfd,filename_buf);
			//printf("transfer_state=%d\n",transfer_state);

			if( transfer_state == 1 )
			{	
				printf("文件:%s完成下载.\n",filename_buf);
				close(m_connectfd);
				m_connectfd = -1;
				continue;
			}
			else if( transfer_state == 0 )
			{
				printf("下载中断.\n");
				close(m_connectfd);	
				m_connectfd = -1;
				continue;
			}
			else //传输前出错 return -1
	        {
	        	printf("传输出错.\n");
	        	close(m_connectfd); 
				m_connectfd = -1;
	           	continue;
	       	 }
		}
		//上传文件
		else if( strncmp(command_buf,"upload",sizeof("download"))==0 )
		{
			int transfer_state = sw_server_uploadfile(m_connectfd,filename_buf);
			//printf("transfer_state=%d\n",transfer_state);

	        if( transfer_state == 1 )
			{
	           	printf("文件:%s完成上传.\n",filename_buf);
				close(m_connectfd);
				m_connectfd = -1;
				continue;
	        }
			else if( transfer_state == 0)
	        {   
	      		printf("上传中断.\n");
	            close(m_connectfd); 
				m_connectfd = -1;
	            continue;
	        }   
	        else //传输前出错 return -1
	        {
	        	printf("传输出错.\n");
	        	close(m_connectfd); 
				m_connectfd = -1;
	            continue;
	        }
		}
		//指令错误 return -1 
		else
		{
			printf("命令错误.请使用upload或者download.\n");
	        close(m_connectfd); 
			m_connectfd = -1;
			continue;;
		}

	}//while(1)	

	close(m_connectfd);
	close(m_socketfd);
	m_connectfd = -1;
	m_socketfd = -1;
	return 0;
}
