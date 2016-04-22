/**
  *@file swsocketfunc.c
  *@brief  实现文件传输的各种函数
  *@author WuQingwen
  *@history
  *         2016-04-10 WuQingwen created
  */
#include "swmysocket.h"

/*
 * 从src拷贝字符串到大小siz的dst中
 * 最多拷贝siz-1个字符， Always NUL terminates (unless siz == 0).
 * 返回strlen(src); 如果retval >= siz, 发生截断.
*/
size_t strlcpy( char *dst, const char *src, size_t siz )
{
    char* d = dst;
    const char* s = src;
    size_t n = siz;
    if ( s == 0 || d == 0 )
         return 0;
    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0)
    {
        do
        {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }
    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';                /* NUL-terminate dst */
        while (*s++)
            ;
    }
    return(s - src - 1);        /* count does not include NUL */
}

/*
 *追加src到siz大小的dst，最多siz-1个字符被追加。
 *返回retval = strlen(dst)+strlen(src)
 *如果retval > siz,则发生了截断
*/
size_t strlcat( char* dst, const char* src, size_t siz )
{
    char* d = dst;
    const char* s = src;
    size_t n = siz;
    size_t dlen;
    if ( s == 0 || d == 0 ) 
        return 0;
    while (n-- != 0 && *d != '\0')
    {
        d++;
    }
    dlen = d - dst;
    n = siz - dlen;
    if (n == 0)
    {
        return(dlen + strlen(s));
    }
    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';
    return(dlen + (s - src));
}


/*
 *发送字符串，成功返回1；失败返回-1；
 */
int sw_send_string(int socketfd,char *string)
{
	if( string ==  NULL)
	{
		return -1;
	}
	int len =  strlen(string)+1;
	if( write(socketfd,&len,sizeof(int))<0 )
	{
		return -1;
	}
	if( write(socketfd,string,len)<0 )
	{
		return -1;
	}
	return 1;
}


/**
  *实现从服务器下载功能，从服务端发送文件filename_buf到客户端
  *return 1,完成下载;0,中断下载 ;-1,出错
  */
int sw_server_downloadfile(int m_connectfd,char *filename_buf)
{	
	if( filename_buf == NULL )
	{
		return -1;
	}
	//接收源地址
	int srcpath_len = 0;
	if( read(m_connectfd,&srcpath_len,sizeof(int))<0 )
	{
		printf("read file_path failed.\n");
		return -1;
	}
	//SSprintf("srcpath_len=%d\n",srcpath_len);
	char src_file_path[FILENAME_SIZE];
 	memset(src_file_path,0,FILENAME_SIZE);
	if( read(m_connectfd,src_file_path,srcpath_len)<0 )
	{
		printf("read file_path failed.\n");
		return -1;
	}
	printf("src file path:%s\n",src_file_path);
	
	FILE* downloadfile_fp = fopen(src_file_path,"r");
	if( downloadfile_fp == NULL )
	{
		perror(src_file_path);
		return -1;
	}
	else
	{
		//计算文件长度
		struct stat srcfile_stat;
		if( stat(src_file_path,&srcfile_stat)==-1)
		{
			perror(src_file_path);
			fclose(downloadfile_fp);
			downloadfile_fp = NULL;
			return -1;
		}
		long long int file_length=srcfile_stat.st_size;
	
		//发送文件长度
		if( write(m_connectfd,&file_length,sizeof(int))<0 )
		{
			printf("send file_length fail.\n");
			fclose(downloadfile_fp);
			downloadfile_fp = NULL;
			return -1;
		}

		//传输文件
		char sendbuf[SEND_SIZE];
		memset(sendbuf,0,sizeof(sendbuf));
		int transfer_length=0;
		int send_length =0;
		long long int transfer_sum=0;
		while( (transfer_length=fread(sendbuf,1,SEND_SIZE,downloadfile_fp))>0 )	
		{
			if( (send_length=send(m_connectfd,sendbuf,transfer_length,MSG_NOSIGNAL))<0 )
			{
				perror(filename_buf);
				break;
			}
			memset(sendbuf,0,sizeof(sendbuf));
			transfer_sum +=send_length;
		}

		fclose(downloadfile_fp);
		downloadfile_fp = NULL;
		//printf("transfer_length=%d\n",transfer_length);
		if( transfer_sum == file_length )//完成下载
			return 1;
	}
	return 0; //中断上传
}

/**
  *实现上传到服务器功能，从客户端发送文件filename_buf到服务端
  *return 1,完成上传;0,中断上传 ;-1,出错
  */
int sw_server_uploadfile(int m_connectfd,char *filename_buf)
{	
	if( filename_buf == NULL )
	{
		return -1;
	}
	//接收目的地址
	int dstpath_len = 0;
	if( read(m_connectfd,&dstpath_len,sizeof(int))<0 )
	{
		printf("write file_path len failed.\n");
		return -1;
	}
	//printf("dstfath_len=%d\n",dstpath_len);
	char dst_file_path[FILENAME_SIZE];
 	memset(dst_file_path,0,FILENAME_SIZE);
	if( read(m_connectfd,dst_file_path,dstpath_len)<0 )
	{
		printf("write file_path failed.\n");
		return -1;
	}
	printf("dst file path:%s\n",dst_file_path);

		
	FILE *uploadfile_fp = fopen(dst_file_path,"w");
	if( uploadfile_fp == NULL )
	{
		printf("open upload file failed.\n");
		return -1;
	}
	else
	{
		//接收文件长度
		long long int file_length = 0;
		if( read(m_connectfd,&file_length,sizeof(int))<0 )
		{
			perror("read file_length");
        	fclose(uploadfile_fp);
        	uploadfile_fp = NULL;
			return -1;
		}
		if( file_length == 0 )
		{
			unlink(dst_file_path);
			return -1;
		}		

		//上传文件到服务器
        char recvbuf[RECV_SIZE];
       	memset(recvbuf,0,sizeof(recvbuf));
       	int transfer_length = 0;
		long long int transfer_sum=0;
       	while((transfer_length=recv(m_connectfd,recvbuf,RECV_SIZE,0))>0)
       	{   
			if( fwrite(recvbuf,sizeof(char),transfer_length,uploadfile_fp)<0 )         
			{
        		printf("file upload failed.\n");
        		break;
       		 }       
           	memset(recvbuf,0,sizeof(recvbuf));
			transfer_sum +=transfer_length;
        } 
        fclose(uploadfile_fp);
        uploadfile_fp = NULL;
        if( transfer_sum == file_length ) 	//完成上传
			return 1;
	}
	return 0; //中断上传
}

/**
  *实现从服务器下载功能，从服务端source_file_path路径发送文件
  *到客户端dest_file_path路径中
  *return 1，完成下载；0，中断下载；-1,错误
  */
int sw_client_downloadfile(int m_socketfd,
						char *source_file_path,
						char *dest_file_path)
{
	if( source_file_path == NULL || dest_file_path == NULL )
	{
		return -1;
	}
	char *src_file_path = source_file_path;
	char *dst_file_path = dest_file_path;
	//发送源地址
	int srcpath_len = strlen(src_file_path)+1;
	printf("srcpath_len=%d\n",srcpath_len);
	if( write(m_socketfd,&srcpath_len,sizeof(int))<0 )
    {
		printf("write file_path len failed.\n");
		return -1;
    }
	//printf("srcpath_len=%d\n",srcpath_len);
	if( write(m_socketfd,src_file_path,srcpath_len)<0 )
    {
		printf("write file_path failed.\n");
		return -1;
    }

	FILE *downloadfile_fp = fopen(dst_file_path,"w");
    if(downloadfile_fp == NULL)
    {
		printf("open download dst file failed.\n");
		return -1;
	}
	else
    {
		//接收文件长度
		long long int file_length = 0;
		if( read(m_socketfd,&file_length,sizeof(int))<0 )
		{
			printf("recive file length fail.\n");
        	fclose(downloadfile_fp);
        	downloadfile_fp = NULL;
			return -1;
		}
		printf("file length = %lld\n",file_length);
		if( file_length == 0 )
		{
			printf("服务端没有该文件.\n");
			return -1;
		}
		//下载文件
        char recvbuf[RECV_SIZE];
        memset(recvbuf,0,sizeof(recvbuf));
       	int transfer_length = 0;
		long long int transfer_sum = 0;
		int percent = 0;
        while((transfer_length=recv(m_socketfd,recvbuf,sizeof(recvbuf),0))>0)
        {
        	if( fwrite(recvbuf,sizeof(char),transfer_length,downloadfile_fp)==-1 )
            {
            	printf("file download failed.\n");
               	break;
           	}
            memset(recvbuf,0,sizeof(recvbuf));
			transfer_sum +=transfer_length;
			//计算进度
			percent=(transfer_sum & 0xff00000000000000)
				?transfer_sum/(file_length/100)
				:transfer_sum*100/file_length;
			
			printf("\rfinish:%d%%",percent);
			fflush(stdout);
				
		}
		printf("\n");
        fclose(downloadfile_fp);
        downloadfile_fp = NULL;
		//printf("tranfer_length = %d\n",transfer_length);
		if( percent == 100 )
		{
			return 1;
		}
    }

    return 0;
}

/**
  *实现上传服务器功能，从客户端路径source_file_path发送文件
  *到服务端路径dest_file_path中
  *return 1，完成上传；0，中断上传 ;-1,出错
  */
int sw_client_uploadfile(int m_socketfd,
						char *source_file_path,
						char *dest_file_path)
{
	if( source_file_path == NULL || dest_file_path == NULL )
	{
		return -1;
	}
	char *src_file_path = source_file_path;
	char *dst_file_path = dest_file_path;

    //printf("src file path:%s\n",src_file_path);
    //printf("dst file path:%s\n",dst_file_path);
	//发送目的地址
	int dstpath_len = strlen(dst_file_path)+1;
	if( write(m_socketfd,&dstpath_len,sizeof(int))<0 )
    {
		printf("write file_path len failed.\n");
		return -1;
    }
	//printf("dstpath_len=%d\n",dstpath_len);
	if( write(m_socketfd,dst_file_path,dstpath_len)<0 )
    {
		printf("write file_path failed.\n");
		return -1;
    }

	FILE *uploadfile_fp = fopen(src_file_path,"r");
    if(uploadfile_fp == NULL)
    {
       	printf("本地找不到文件.\n");
       	return -1;
    }
	else
   	{
		//计算文件长度
		struct stat srcfile_stat;
		if( stat(src_file_path,&srcfile_stat)==-1 )
		{
			perror(src_file_path);
       		fclose(uploadfile_fp);
        	uploadfile_fp = NULL;
			return -1;
		}	
		long long int file_length=srcfile_stat.st_size;
        
		//发送文件长度
		if( write(m_socketfd,&file_length,sizeof(int))<0 )
		{
			perror("send file length");
        	fclose(uploadfile_fp);
        	uploadfile_fp = NULL;
			return -1;
		}
		//上传文件
        char sendbuf[SEND_SIZE];
        memset(sendbuf,0,sizeof(sendbuf));
       	int transfer_length=0;
        int write_length=0;
        long long int transfer_sum = 0;
        int percent = 0;
        while( (transfer_length=fread(sendbuf,sizeof(char),sizeof(sendbuf),uploadfile_fp))>0 ) 
        {   
			if( (write_length=write(m_socketfd,sendbuf,transfer_length))<0 )
            {   
				printf("file upload fail.\n");
                break;
            }   
            memset(sendbuf,0,sizeof(sendbuf));
            transfer_sum +=write_length;
            //计算进度
            percent=(transfer_sum & 0xff00000000000000)
                	?transfer_sum/(file_length/100)
                	:transfer_sum*100/file_length;
            printf("\rfinished:%d%%",percent);
			fflush(stdout);		
        }
        printf("\n");   
        fclose(uploadfile_fp);
        uploadfile_fp = NULL;
        if( transfer_sum == 0 )
        {
        	printf("没有该文件.\n");
        	return -1;
        }
        if( percent == 100 )
		{
			return 1;
		}
	}

	return 0;
}
