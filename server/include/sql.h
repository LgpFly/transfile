#ifndef __SQL_H__
#define __SQL_H__

#include "head.h"
#include "userinfo.h"

/*
 * 连接数据库 
*/
int sqlConnect(MYSQL**);

void createSalt(char*, int);

void createToken(char*, int);

int findUserByName(MYSQL*, char*);

// 登录时把用户信息填到结构体中
int getUserInfo(MYSQL* sql_conn, char* name,  UserInfo* user_info);

int addRootFile(MYSQL* sql_conn, char* u_name, char* f_name, char type, int f_level, int f_level_dad);

void addDir(MYSQL* sql_conn, char* u_name, char* f_name, char type, int f_level, int f_level_dad);

int findDir(MYSQL*, char*, char*, int);

int myChDir(MYSQL*, UserInfo*, int, char (*dir)[20]);

#endif
