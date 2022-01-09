#include "../include/epoll.h"
#include "../include/userinfo.h"

int epollAddFd(int epfd, int client_fd){
    int ret;
    struct epoll_event ev;
    bzero(&ev, sizeof(struct epoll_event));
    ev.data.fd = client_fd;
    ev.events = EPOLLIN;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
    ERROR_CHECK(ret, -1, "epoll_ctl(add)");
    return 0;
}

int epollDelFd(int epfd, int client_fd){
    int ret;
    struct epoll_event ev;
    bzero(&ev, sizeof(struct epoll_event));
    ev.data.fd = client_fd;
    ev.events = EPOLLIN;
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
    ERROR_CHECK(ret, -1, "epoll_ctl(de;)");
    return 0;
}

void closeConn(int epfd, UserInfo* user_info, int m){
    printf("client dont conn\n");
    epollDelFd(epfd, user_info[m].main_socket_fd);
    close(user_info[m].main_socket_fd);
    memset(user_info + m, 0, sizeof(UserInfo));

}
