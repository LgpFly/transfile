#ifndef __USERINFO_H__
#define __USERINFO_H__

#include "head.h"

typedef struct{
    char u_name[20];            // username
    char u_pswd[30];            // pswd
    char u_token[30];           // token
    char u_salt[3];
    char u_path[50];            // current path
    int main_socket_fd;         // 客户端第一次连接的socketfd
    int f_level;                // 当前目录层级
    int f_level_dad;            // 父目录层级
    int con_flag;               // 连接状态
    int login_flag;
    char dir_dad[20];
}UserInfo, *pUserInfo;


/*
 * 注册添加用户,写入数据库
 * 0:success, -1:fail重名(后面的每个函数都是这样)
*/
int addUser(MYSQL*, char*, char*, char*, char*);

void initUserInfo(pUserInfo);

void chUserInfo(UserInfo*, int, char (*dir)[20]);

#endif
