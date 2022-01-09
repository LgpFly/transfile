#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "head.h"
#include "userinfo.h"

// 添加监听描述符
int epollAddFd(int, int);
// 删除监听描述符
int epollDelFd(int, int);

void closeConn(int, UserInfo*, int);

#endif
