#include "../include/head.h"
#include "../include/transfile.h"
#include "../include/userinfo.h"
#include "../include/epoll.h"

UserInfo user_info;

void* threadFund(void* p_arg){
    int ret;
    int con_flag = 1;
    struct sockaddr_in* cli_child_addr = (struct sockaddr_in*)p_arg; 
    int child_fd = socket(AF_INET, SOCK_STREAM, 0);

    ret = connect(child_fd, (struct sockaddr*)cli_child_addr, sizeof(struct sockaddr_in));
    ERROR_CHECK(ret, -1, "connect");
    ret = send(child_fd, &con_flag, 4, 0);
    ERROR_CHECK(ret, -1, "send con_flag");
    ret = recvFile(child_fd);
    if(-1 == ret){
#ifdef _DEBUG
        printf("server fly\n");
#endif
    }
    if(0 == ret){
        printf("download file success\n");
    }
    
}


int main(int argc, char **argv){
    ARGS_CHECK(argc, 3);
    int con_flag = -1;
    int ret;
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(socket_fd, -1, "socket");
    
    struct sockaddr_in cli_addr;
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(atoi(argv[2]));
    cli_addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    memset(&user_info, 0, sizeof(user_info));

    ret = connect(socket_fd, (struct sockaddr*)&cli_addr, sizeof(cli_addr));
    ERROR_CHECK(ret, -1, "connect");
    ret = send(socket_fd, &con_flag, 4, 0);
    ERROR_CHECK(ret, -1, "send con_flag");
    // pthread_t pid;
    // pthread_create(&pid, NULL, threadFund, &cli_addr);
    int epfd;
    epfd = epoll_create(1);
    ret = epollAddFd(epfd, 0);
    ERROR_CHECK(ret, -1, "epollAdd 0");
    ret = epollAddFd(epfd, socket_fd);
    ERROR_CHECK(ret, -1, "epollAdd socket_fd");
    
    char cur_buf[20];
    char cur_name[20];
    int cur_name_len;
    char cur_salt[3];
    char cur_secret[30];
    char cur_token[30];
    int order;
    char *pswd = (char*) calloc(8, 1);
    Train train;
    int type;
    int result;
    int cur_secret_len;
login_begin:
    system("clear");
    printf("请选择注册或者登陆：\n1:注册新账号\n2:已有账号直接登陆\n");
    scanf("%d",&type);
    bzero(cur_buf, sizeof(cur_buf));
    if(1 == type)
    {

        printf("请输入要注册的用户名\n");
        bzero(cur_name, sizeof(cur_name));
        scanf("%s", cur_name);
        bzero(pswd, sizeof(pswd));
        pswd = getpass("请输入密码：");
        bzero(cur_buf, sizeof(cur_buf));
        strcpy(cur_buf, pswd);
        bzero(pswd, sizeof(pswd));
        pswd = getpass("请再次输入密码确认：");
        // scanf("%s", pswd);
        if(strcmp(cur_buf,pswd)!=0)
        {
            printf("两次输入不一致，即将回到主界面\n");
            sleep(1);
            goto login_begin;
        }else
        {
            // 发送指令post
            order = 1;
            ret = send(socket_fd, &order, 4, 0);
            ERROR_CHECK(ret, -1, "send post");

            // send user_name
            // cur_name_len = strlen(cur_name);
            // ret = send(socket_fd, &cur_name_len, 4, 0);
            // ERROR_CHECK(ret, -1, "send cur_name_len");
            ret = send(socket_fd, cur_name, 20, 0);
            ERROR_CHECK(ret, -1, "send post");
            
            // 有无重名状态码
            int have_user;
            ret = recv(socket_fd, &have_user, 4, 0);
            if(0 == ret){
                printf("server dont conn\n");
                close(socket_fd);
                exit(0);
            }
            if(0 == have_user){
                // 接收salt值
                bzero(cur_salt, sizeof(cur_salt));
                ret = recv(socket_fd, cur_salt, 2, 0);
                if(0 == ret){
                    printf("server dont conn\n");
                    close(socket_fd);
                    exit(0);
                }

                // 生成密文，发送给服务器
                bzero(cur_secret, sizeof(cur_secret));
#ifdef _DEBUG
                printf("pswd:%s, salt:%s\n", pswd, cur_salt);

#endif
                memcpy(cur_secret, crypt(pswd, cur_salt), strlen(crypt(pswd, cur_salt)));
#ifdef _DEBUG
                printf("memcpy secret:%s\n", cur_secret);
                printf("secret:%s\n", crypt(pswd, cur_salt));
#endif
                fflush(stdout);
                ret = send(socket_fd, cur_secret, 30, 0);
                ERROR_CHECK(ret, -1, "send secret");
                printf("发送字节数：%d\n", ret);
                ret = recv(socket_fd, &result, 4, 0);
                if(0 == ret){
                    printf("server dont conn\n");
                    close(socket_fd);
                    exit(0);
                }
                if(0 == result){
                    printf("post success, renew\n");
                    sleep(1);
                    goto login_begin;
                }else{
                    printf("write sql fail\n");
                    sleep(1);
                    goto login_begin;
                }

            }else if(-1 == have_user){
                printf("user_name has been used\n");
                sleep(1);
                goto login_begin;
            }
            
            

        }
            
    }else if(2 == type){
        /* 登录 */
        // 发送登录的order
        order = 2;
        ret = send(socket_fd, &order, 4, 0);
        ERROR_CHECK(ret, -1, "send login");
        // 输入并发送用户名
        printf("请输入用户名:");
        bzero(cur_name, sizeof(cur_name));
        scanf("%s", cur_name);
        ret = send(socket_fd, cur_name, 20, 0);
        ERROR_CHECK(ret, -1, "send user_name");
        
        // 接收是否有用户的状态
        ret = recv(socket_fd, &result, 4, 0);
        if(0 == ret){
            printf("server dont conn\n");
            close(socket_fd);
            exit(0);
        }
        if(0 == result){
            // 有用户
            bzero(pswd, 8);
            pswd = getpass("请输入密码:\n");
            // recv salt;
            bzero(cur_salt, sizeof(cur_salt));
            ret = recv(socket_fd, cur_salt, 2, MSG_WAITALL);
            if(0 == ret){
                printf("server dont conn\n");
                close(socket_fd);
                exit(0);
            }
            // create secret send to server;
            bzero(cur_secret, sizeof(cur_secret));
            strcpy(cur_secret, crypt(pswd, cur_salt));
            ret = send(socket_fd, cur_secret, 30, 0);
            if(-1 == ret){
                printf("server dont conn\n");
                close(socket_fd);
                exit(0);
            }
            // recv result;
            ret = recv(socket_fd, &result, 4, MSG_WAITALL);
            if(-1 == ret){
                printf("server dont conn\n");
                close(socket_fd);
                exit(0);
            }
            if(0 == result){
                // login success
                // recv token
                bzero(cur_token, sizeof(cur_token));
                ret = recv(socket_fd, cur_token, 30, MSG_WAITALL);
                if(-1 == ret){
                    printf("server dont conn\n");
                    close(socket_fd);
                    exit(0);
                }
                // init user_info struct
                initUserInfo(&user_info, cur_name, cur_token);
                printf("login success\n");
            }else if(-1 == result){
                // login error
                printf("pswd error\n");
                goto login_begin;
            }
        }else{
            // 此用户不存在
            printf("此用户不存在\n");
            sleep(1);
            goto login_begin;

        }
    }else{
        printf("input error!\n");
        goto login_begin;
    }

    while(1){


    }


}
