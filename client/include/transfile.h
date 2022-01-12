#ifndef __TRANSFILE_H__
#define __TRANSFILE_H__

#include "head.h"

typedef struct{
    int data_len;
    char buf[1020];
}Train;

typedef struct{
    char f_name[20];
    long f_size;
    char f_md5[33];
}UpFileInfo, *pUpFileInfo;


int getFileInfo(char*, pUpFileInfo);

// 发送一个文件
int sendFile(int client_fd, char* name, long size);

// 接收一个文件
int recvFile(int client_fd);

#endif
