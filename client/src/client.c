#include "../include/head.h"
#include "../include/transfile.h"
#include "../include/userinfo.h"
#include "../include/epoll.h"
#include "../include/order.h"
UserInfo user_info;
struct sockaddr_in cli_addr;
int socket_fd;

// void* threadFund(void* p_arg){
//     int ret;
//     int con_flag = 1;
//     struct sockaddr_in* cli_child_addr = (struct sockaddr_in*)p_arg; 
//     int child_fd = socket(AF_INET, SOCK_STREAM, 0);
// 
//     ret = connect(child_fd, (struct sockaddr*)cli_child_addr, sizeof(struct sockaddr_in));
//     ERROR_CHECK(ret, -1, "connect");
//     ret = send(child_fd, &con_flag, 4, 0);
//     ERROR_CHECK(ret, -1, "send con_flag");
//     ret = recvFile(child_fd);
//     if(-1 == ret){
// #ifdef _DEBUG
//         printf("server fly\n");
// #endif
//     }
//     if(0 == ret){
//         printf("download file success\n");
//     }
//     
// }

void* downsFunc1(void* p_arg){

    pDownFileInfo p_pice_info = (pDownFileInfo)p_arg;
    int ret;
    int con_flag = 1;
    char file_name[20];
    bzero(file_name, sizeof(file_name));
    strcpy(file_name, p_pice_info->f_name);
    int download_fd = socket(AF_INET, SOCK_STREAM, 0);
    ret = connect(download_fd, (struct sockaddr*)&cli_addr, sizeof(struct sockaddr_in));
    ret = send(download_fd, &con_flag, 4, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    ret = send(download_fd, &(user_info.main_socket_fd), 4, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    int up_or_down = 3;
    ret = send(download_fd, &up_or_down, 4, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);

    // 发送要下载的文件名
    ret = send(download_fd, file_name, 20, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    // 发送文件偏移位置
    ret = send(download_fd, &p_pice_info->f_seek, 8, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    // 发送文件要读取的大小
    ret = send(download_fd, &p_pice_info->recv_size, 8, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    // 接收文件大小先，然后写到目录中去
    // long file_res;
    // ret = recv(download_fd, &file_res, 8, MSG_WAITALL);
    // CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
#ifdef _DEBUG
    printf("f_sizeL%ld\n", p_pice_info->recv_size);
#endif
    // if(-1 == file_res){
        // printf("no this file\n");
        // close(download_fd);
    // }else{
        ret = recvsFile(download_fd, file_name, p_pice_info->recv_size, p_pice_info->f_seek);
        if(0 == ret){
            printf("download success\n");
        }else{
            printf("unknown error\n");
        }
    // }
}



void* downloadFunc(void* p_arg){
    
    int ret;
    int con_flag = 1;
    char file_name[20];
    bzero(file_name, sizeof(file_name));
    strcpy(file_name, p_arg);
    int download_fd = socket(AF_INET, SOCK_STREAM, 0);
    ret = connect(download_fd, (struct sockaddr*)&cli_addr, sizeof(struct sockaddr_in));
    ret = send(download_fd, &con_flag, 4, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    ret = send(download_fd, &(user_info.main_socket_fd), 4, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
    int up_or_down = 2;
    ret = send(download_fd, &up_or_down, 4, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);

    // 发送要下载的文件名
    ret = send(download_fd, file_name, 20, 0);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);

    // 接收文件写到目录中去
    long file_res;
    ret = recv(download_fd, &file_res, 8, MSG_WAITALL);
    CHILD_CHECK_RET(-1, ret, socket_fd, download_fd);
#ifdef _DEBUG
    printf("f_sizeL%ld\n", file_res);
#endif
    if(-1 == file_res){
        printf("no this file\n");
        close(download_fd);
    }else{
        ret = recvFile(download_fd, file_name, file_res);
        if(0 == ret){
            printf("download success\n");
        }else{
            printf("unknown error\n");
        }
    }
}



void *uploadFunc(void* p_arg){
    int ret;
    int con_flag = 1;
    pUpFileInfo p_file_info = (pUpFileInfo)p_arg;
#ifdef _DEBUG
    printf("new thread %d, port:%d\n", user_info.main_socket_fd, ntohs(cli_addr.sin_port));
#endif
    int upload_fd = socket(AF_INET, SOCK_STREAM, 0);
    ret = connect(upload_fd, (struct sockaddr*)&cli_addr, sizeof(struct sockaddr_in));
    ret = send(upload_fd, &con_flag, 4, 0);
    if(-1 == ret){
        printf("fly\n");
        close(upload_fd);
        close(socket_fd);
        exit(0);
    }
    // 发送main_socket_fd，使server知道是哪个客户端
    ret = send(upload_fd, &user_info.main_socket_fd, 4, 0);
    if(-1 == ret){
        printf("fly\n");
        close(upload_fd);
        close(socket_fd);
        exit(0);
    }
    // 发送上传还是下载命令1：上传 2：普通下载 3：vip下载
    int up_or_down = 1;
    ret = send(upload_fd, &up_or_down, 4, 0);
    if(-1 == ret){
        printf("fly\n");
        close(upload_fd);
        close(socket_fd);
        exit(0);
    }
    
    // 发送文件信息
    ret = send(upload_fd, p_file_info->f_name, 20, 0);
    if(-1 == ret){
        printf("fly\n");
        close(upload_fd);
        close(socket_fd);
        exit(0);
    }
    ret = send(upload_fd, &(p_file_info->f_size), 8, 0);
    if(-1 == ret){
        printf("fly\n");
        close(upload_fd);
        close(socket_fd);
        exit(0);
    }
    ret = send(upload_fd, p_file_info->f_md5, 33, 0);
    if(-1 == ret){
        printf("fly\n");
        close(upload_fd);
        close(socket_fd);
        exit(0);
    }

    sendFile(upload_fd, p_file_info->f_name, p_file_info->f_size);  

}


int main(int argc, char **argv){
    ARGS_CHECK(argc, 3);
    int con_flag = -1;
    int ret;
    // int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(socket_fd, -1, "socket");
    
    // struct sockaddr_in cli_addr;
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
    
    struct epoll_event evs[10];

    char cur_buf[20];
    char cur_name[20];
    int cur_name_len;
    char cur_salt[3];
    char cur_secret[30];
    char cur_token[30];
    char cur_path[50];
    int f_level;
    int f_level_dad;
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
                // bzero(cur_path, sizeof(cur_path));
                // ret = recv(socket_fd, cur_path, 50, MSG_WAITALL);
                // if(-1 == ret){
                //     printf("server dont conn\n");
                //     close(socket_fd);
                //     exit(0);
                // }
                ret = recv(socket_fd, &f_level, 4, MSG_WAITALL);
                if(-1 == ret){
                    printf("server dont conn\n");
                    close(socket_fd);
                    exit(0);
                }
                ret = recv(socket_fd, &f_level_dad, 4, MSG_WAITALL);
                if(-1 == ret){
                    printf("server dont conn\n");
                    close(socket_fd);
                    exit(0);
                }
                int main_socket_fd;
                ret = recv(socket_fd, &main_socket_fd, 4, 0);
                CHECK_RET(ret, -1, socket_fd);
                // init user_info struct
                initUserInfo(&user_info, cur_name, cur_token, f_level, f_level_dad, main_socket_fd);
                printf("main_socket_fd:%d\n", user_info.main_socket_fd);
                printf("login success\n");
#ifdef _DEBUG
                printf("%s, %s, %s, %d, %d\n", user_info.u_name, user_info.u_path,
                       user_info.u_token, user_info.f_level, user_info.f_level_dad);
#endif
                print(user_info.u_path);
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
    
    char cur_order[40];
    while(1){
        int ready_num = epoll_wait(epfd, evs, 10, 0);
        for(int i = 0; i < ready_num; ++i){
            if(evs[i].data.fd == 0){
                memset(cur_order, 0, sizeof(cur_order));
                setbuf(stdin, NULL);
                gets(cur_order);
                char first[40];
                char second[20];
                memset(first, 0, sizeof(first));
                memset(second, 0, sizeof(second));
                orderSplit(cur_order, first, second);
#ifdef _DEBUG
                printf("order:%s,first:%s,second:%s\n", cur_order, first, second);
#endif
                
                if(0 == strcmp(first, "mdir")){
                    // 发送mdir的指令
                    if(0 != strlen(second)){
                        order = 11;
                        ret = send(socket_fd, &order, 4, 0);
                        ERROR_CHECK(ret, -1, "send 11");
                        ret = send(socket_fd, second, 20, 0);
                        ERROR_CHECK(ret, -1, "send 11 second");
                        int mkdir_result;
                        ret = recv(socket_fd, &mkdir_result, 4, 0);
                        if(0 == ret){
                            close(socket_fd);
                            printf("server fly\n");
                            exit(0);
                        }
                        if(0 == mkdir_result){
#ifdef _DEBUG
                            printf("mkdir success\n");
#endif
                            print(user_info.u_path);
                        }else if(-1 == mkdir_result){
                            printf("%s has exit, please change name!\n", second);
                            print(user_info.u_path);
                        }
                    }else{
                        printf("order error\n");
                        print(user_info.u_path);
                    }

                }else if(0 == strcmp(first, "cd")){
                    if(0 != strlen(second)){
                        order = 4;
                        ret = send(socket_fd, &order, 4, 0);
                        if(-1 == ret){
                            close(socket_fd);
                            printf("server fly\n");
                            exit(0);
                        }
                        // 具体指令
                        ret = send(socket_fd, second, 20, 0);
                        if(-1 == ret){
                            close(socket_fd);
                            printf("server fly\n");
                            exit(0);
                        }
                        int result;
                        ret = recv(socket_fd, &result, 4, 0);
                        if(0 == ret){
                            printf("fly\n");
                            close(socket_fd);
                            exit(0);
                        }
                        if(0 == result){
                            // 成功到达路径
                            char cur_dir[5][20];
                            bzero(cur_dir, sizeof(cur_dir));
                            dirSplit(second, cur_dir);
                            chPath(&user_info, cur_dir);
                            print(user_info.u_path);
                        }else{
                            printf("dir error\n");
                            print(user_info.u_path);
                        }

                    }else{
                        printf("order error!\n");
                        print(user_info.u_path);
                    }
                }else if(0 == strcmp(first, "ls")){
                    
                    order = 3;
                    ret = send(socket_fd, &order, 4, 0);
                    if(-1 == ret){
                        printf("fly\n");
                        close(socket_fd);
                        exit(0);        
                    }
                    int ls_res;
                    char ls_result[100];
                    ret = recv(socket_fd, &ls_res, 4, 0);
                    if(0 == ret){
                        printf("fly\n");
                        close(socket_fd);
                        exit(0);        
                    }
                    if(0 == ls_res){
                        memset(ls_result, 0, sizeof(ls_result));
                        ret = recv(socket_fd, ls_result, 100, 0);
                        if(0 == ret){
                            printf("fly\n");
                            close(socket_fd);
                            exit(0);        
                        }
                        printf("%s", ls_result);
                        print(user_info.u_path);
                    }else if(-1 == ls_res){
                        printf("unknow error\n");
                        print(user_info.u_path);
                    }
                    
                }else if(0 == strcmp(first, "upload")){
                    // 上传文件
                    order = 5;
                    ret = send(socket_fd, &order, 4, 0);                    
                    CHECK_RET(-1, ret, socket_fd);
                    UpFileInfo file_info;
                    memset(&file_info, 0, sizeof(file_info));
                    ret = getFileInfo(second, &file_info);
                    if(-1 == ret){
                        printf("file error\n");
                        print(user_info.u_path);
                    }else{
                        // printf("我会新建一个线程来上传这个文件\n");
                        pthread_t p_id;
                        ret = pthread_create(&p_id, NULL, uploadFunc, &file_info);
                        print(user_info.u_path);
                    }
                }else if(0 == strcmp(first, "download")){
                    // 普通下载命令
                    order = 6;
                    ret = send(socket_fd, &order, 4, 0);
                    CHECK_RET(-1, ret, socket_fd);
                    pthread_t down_pid;
                    ret = pthread_create(&down_pid, NULL, downloadFunc, second);

                }else if(0 == strcmp(first, "downloads")){
                    order = 7;
                    ret = send(socket_fd, &order, 4, 0);
                    CHECK_RET(-1, ret, socket_fd);

                    // 发送文件名
                    ret = send(socket_fd, second, 20, 0);
                    CHECK_RET(-1, ret, socket_fd);

                    // 接收文件大小
                    long file_size;
                    ret = recv(socket_fd, &file_size, 8, 0);
                    CHECK_RET(-1, ret, socket_fd);
                    if(-1 == file_size){
                        printf("no this file\n");
                        print(user_info.u_path);
                    }else{
#ifdef _DEBUG
                        printf("flie_size:%ld\n", file_size);
#endif
                        long one_pice = file_size / 3; 
                        DownFileInfo down_file_info[3];
                        bzero(down_file_info, sizeof(DownFileInfo) * 3);
                        // 先对第一个结构体赋值
                        strcpy(down_file_info[0].f_name, second);
                        down_file_info[0].f_seek = 0;
                        down_file_info[0].recv_size = one_pice;

                        // 第二个
                        strcpy(down_file_info[1].f_name, second);
                        down_file_info[1].f_seek = one_pice;
                        down_file_info[1].recv_size = one_pice;

                        // 第三个
                        
                        strcpy(down_file_info[2].f_name, second);
                        down_file_info[2].f_seek = one_pice * 2;
                        down_file_info[2].recv_size = file_size - (one_pice * 2);

#ifdef _DEBUG
                        printf("here\n");
                        for(int i = 0; i < 3; ++i)
                            printf("name:%s,f_seek:%ld,recv_size:%ld\n", down_file_info[i].f_name, 
                                   down_file_info[i].f_seek, down_file_info[i].recv_size);
#endif
                        int fd = open(second, O_CREAT|O_RDWR, 0666);
                        ftruncate(fd, file_size);
                        lseek(fd, 0, 0);
                        close(fd);

                        pthread_t downs_pid[3];
                        pthread_create(downs_pid, NULL, downsFunc1, &(down_file_info[0]));
                        pthread_create(downs_pid + 1, NULL, downsFunc1, &(down_file_info[1]));
                        pthread_create(downs_pid + 2, NULL, downsFunc1, &(down_file_info[2]));
                        print(user_info.u_path);
                    }
                }else if(0 == strcmp(first, "del")){
                    order = 9;
                    ret = send(socket_fd, &order, 4, 0);                    
                    CHECK_RET(-1, ret, socket_fd);

                    // 发送要删除的文件名
                    ret = send(socket_fd, second, 20, 0);
                    CHECK_RET(-1, ret, socket_fd);

                    // 接收删除的结果
                    int del_res;
                    ret = recv(socket_fd, &del_res, 4, 0);
                    CHECK_RET(0, ret, socket_fd);

                    if(0 == del_res){
                        printf("del success\n");
                    }else{
                        printf("del error\n");
                    }

                    print(user_info.u_path);

                }else if(0 == strcmp(first, "quit")){
                    order = 10;
                    ret = send(socket_fd, &order, 4, 0);
                    close(socket_fd);
                    printf("bye\n");
                    exit(0);
                }else{
                    printf("order error\n");
                    print(user_info.u_path);
                }

            }
        }

    }


}
