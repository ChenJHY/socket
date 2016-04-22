/**
  *@file swmysocket.h
  *@brief  包含头文件与函数声明
  *@author WuQingwen
  *@history
  *     2016-04-10 WuQingwen created
  */
#ifndef __SWMYSOCKET_H__
#define __SWMYSOCKET_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_PORT     6666
#define FILENAME_SIZE	100
#define SEND_SIZE		1024*8
#define RECV_SIZE		1024*8
#define COMMAND_SIZE  9
/**
  *@brief 实现从服务器下载功能
  *@param[in] m_connectfd  连接描述符
  *@param[in] src_file_path 服务端源地址，从该地址下载
  *@param[in] dst_file_path 客户端目标地址，下载到该地址
  *@return 1，完成下载；0，中断下载；-1,错误
  */
int sw_client_downloadfile(int m_socketfd,
							char *src_file_path,
							char *dst_file_path);

/**
  *@brief 实现上传服务器功能
  *@param[in] m_connectfd  连接描述符
  *@param[in] src_file_path 客户端源地址，从该地址上传文件
  *@param[in] dst_file_path 服务器目标地址，上传到该地址
  *@return 1,完成上传；0，中断上传；-1,错误
  */
int sw_client_uploadfile(int m_socketfd,
						  char *src_file_path,
						  char *dst_file_path);

/**
  *@brief 实现从服务器下载功能
  *@param[in] m_connectfd
  *@param[in] filename_buf
  *@return 1,完成下载;0,中断下载；-1,错误    
  */
int sw_server_downloadfile(int m_connectfd,char *filename_buf);

/**
  *@brief 实现上传到服务器的功能
  *@param[in] m_connectfd
  *@param[in] filename_buf
  *@return 1,完成上传;0,中断上传；-1,错误
  */
int sw_server_uploadfile(int m_connectfd,char *filename_buf);


/**
  *@brief 从src中复制字符串到siz大小的dst
  *@param[in] dst
  *@param[out] src
  *@param[in] siz  sizeof(dst)
  */
size_t strlcpy( char *dst, const char *src, size_t siz );

/**
  *@brief 追加src的字符串到siz大小的dst
  *@param[in] dst
  *@param[out] src
  *@param[in] siz  sizeof(dst)
  */
size_t strlcat( char* dst, const char* src, size_t siz );

/**
  *@brief 发送字符串
  *@param[in] socketfd 套接字
  *@param[out] string   要发送的字符串
  *@return 1，成功；-1 ，失败
  */
int sw_send_string(int socketfd,char *string);


#endif /*__SWMYSOCKET_H__*/
