#ifndef __USERINFO_H__
#define __USERINFO_H__

#include "head.h"

typedef struct{
    char u_name[20];            // username
    char u_token[30];           // token
    char u_path[50];            // current path
    int f_level;                // 当前目录层级
    int f_level_dad;            // 父目录层级
}UserInfo, *pUserInfo;

void initUserInfo(UserInfo*, char*, char*, int, int);


#endif
