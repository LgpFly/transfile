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

// for downloads
typedef struct{
    char f_name[20];
    long f_seek;
    long recv_size;
}DownFileInfo, *pDownFileInfo;



int getFileInfo(char*, pUpFileInfo);

// 发送一个文件
int sendFile(int client_fd, char* name, long size);

// 接收一个文件
int recvFile(int client_fd, char* name, long f_size);

int recvsFile(int, char*, long, long);

#endif
