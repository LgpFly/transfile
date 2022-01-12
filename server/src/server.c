#include "../include/pool.h"
#include "../include/tcp.h"
#include "../include/epoll.h"
#include "../include/userinfo.h"
#include "../include/fileinfo.h"
#include "../include/sql.h"
#include "../include/order.h"

extern MYSQL* sql_conn;


int exit_pipe[2];
void sigFunc(int signum){
#ifdef _DEBUG
    printf("signal is coming\n");
#endif
    write(exit_pipe[1], &signum, 4);
}

// 输入参数ip，port，thread_num，capacity of taskque
int main(int argc, char **argv){

    umask(0000);
    ARGS_CHECK(argc, 5);
    pipe(exit_pipe);

    if(fork()){
        close(exit_pipe[0]);
        signal(SIGUSR1, sigFunc);
        wait(NULL);
        exit(0);
    }
    UserInfo user_info[10];
    memset(user_info, 0, sizeof(user_info));
    
    close(exit_pipe[1]);
    int thread_num = atoi(argv[3]);
    int que_capacity = atoi(argv[4]);
    int ret;
    // 线程池结构体
    ThreadPool thread_pool;
    bzero(&thread_pool, sizeof(ThreadPool));

    // 初始化线程池结构体
    threadPoolInit(&thread_pool, thread_num, que_capacity);
    // 启动线程池
    threadPoolStart(&thread_pool);
   
    // 创建TCP监听套接字
    int socket_fd;
    tcpInit(&socket_fd, argv[1], argv[2]);
    
    int epfd;
    epfd = epoll_create(1);
    ERROR_CHECK(epfd, -1, "epoll_create");
    epollAddFd(epfd, socket_fd);
    epollAddFd(epfd, exit_pipe[0]);
    int ready_num = 0;
    struct epoll_event evs[10];
    bzero(evs, sizeof(evs));
    sqlConnect(&sql_conn);
    
    int order;
    int con_flag;
    int client_fd;
    int cur_len;
    char cur_buf[1020];
    char cur_name[20];
    char cur_salt[3];
    char cur_token[30];
    char cur_secret[30];
    int cur_secret_len;
    char second[20];
    char cwd[50];
    while(1){
        ready_num = epoll_wait(epfd, evs, 10, -1);
        for(int i = 0; i < ready_num; ++i){
            if(evs[i].data.fd == socket_fd){
                client_fd = accept(socket_fd, NULL, NULL);
                ERROR_CHECK(client_fd, -1, "accept");
                ret = recv(client_fd, &con_flag, 4, 0);
                ERROR_CHECK(ret, -1, "recv flag");
                if(0 == ret){
                    printf("child thread dont send con_flag\n");
                }
#ifdef _DEBUG
                printf("con_flag = %d\n", con_flag);
#endif
                if(-1 != con_flag){
                    int up_or_down;
                    int main_fd;
                    char child_name[20];
                    char child_path[50];
                    int child_level;

                    // 找到是哪个客户端
                    ret = recv(client_fd, &main_fd, 4, MSG_WAITALL);
                    if(0 == ret){
                        printf("client dont conn\n");
                        close(client_fd);
                    }
                    int i;
                    for(i = 0; i < 10; ++i){
                        if(main_fd == user_info[i].main_socket_fd){
                            memset(child_name, 0, sizeof(child_name));
                            strcpy(child_name, user_info[i].u_name);
                            memset(child_path, 0, sizeof(child_path));
                            strcpy(child_path, user_info[i].u_path);
                            child_level = user_info[i].f_level;
#ifdef _DEBUG
                            printf("child infomaintion:%s%s%d\n", child_name, child_path, child_level);
#endif
                            break;
                        }
                    }
                    if(10 == i){
                        printf("child unknown error\n ");
                    }
                    
                    // 接收是上传还是下载的信息
                    ret = recv(client_fd, &up_or_down, 4, MSG_WAITALL);
                    if(0 == ret){
                        printf("client dont conn\n");
                        close(client_fd);
                    }
                    
                    // 1:上传文件
                    if(1 == up_or_down){
                        // 创建一个任务节点
                        pQueNode p_node = (pQueNode)calloc(1, sizeof(QueNode));
                        p_node->up_or_down = 1;
                        // 接收文件信息并初始化任务节点
                        ret = recv(client_fd, p_node->f_name, 20, MSG_WAITALL);
                        if(0 == ret){
                            printf("client dont conn\n");
                            close(client_fd);
                        }
                        ret = recv(client_fd, &(p_node->f_size), 8, MSG_WAITALL);
                        if(0 == ret){
                            printf("client dont conn\n");
                            close(client_fd);
                        }
                        ret = recv(client_fd, p_node->f_md5, 33, MSG_WAITALL);
                        if(0 == ret){
                            printf("client dont conn\n");
                            close(client_fd);
                        }
                        strcpy(p_node->u_name, child_name);
                        strcpy(p_node->u_path, child_path);
                        p_node->f_level = child_level;
                        p_node->client_fd = client_fd;
                        
#ifdef _DEBUG
                        printf("p_node information:%d,%s,%s,%d,%s,%s,%ld,%d\n", p_node->up_or_down,
                               p_node->f_name, p_node->f_md5, p_node->f_level, p_node->u_name,
                               p_node->u_path, p_node->f_size, p_node->client_fd);
#endif
                        // 将任务节点放到任务队列中
                        pthread_mutex_lock(&(thread_pool.task_que.mutex));
                        ret = taskQueInsert(&(thread_pool.task_que), p_node);
                        pthread_mutex_unlock(&(thread_pool.task_que.mutex));
                        // 唤醒子线程
                        pthread_cond_signal(&(thread_pool.task_que.cond));

                    } // end of if(1 == up_or_down)

                    // pQueNode p_node = (pQueNode)calloc(1, sizeof(QueNode));
                    // p_node->client_fd = client_fd;
                    // // 加入任务队列中
                    // pthread_mutex_lock(&(thread_pool.task_que.mutex));
                    // ret = taskQueInsert(&(thread_pool.task_que), p_node);
                    // pthread_mutex_unlock(&(thread_pool.task_que.mutex));
                    // // 唤醒子线程
                    // pthread_cond_signal(&(thread_pool.task_que.cond));
                }// endif(-1 != con_flag)
                if(-1 == con_flag){
                    epollAddFd(epfd, client_fd);
                    for(int l = 0; l < 10; ++l){
                        if(0 == user_info[l].con_flag){
                            user_info[l].main_socket_fd = client_fd;
                            user_info[l].con_flag = 1;
                            break;
                        }
                    }
                }
            }// endif(evs[i].data.fd == socket_fd)
            // 通知子线程退出
            else if(evs[i].data.fd == exit_pipe[0]){
#ifdef _DEBUG
                printf("pipe can read\n");
#endif
                thread_pool.task_que.exit_flag = 1;
                // 给所有线程发消息
                pthread_cond_broadcast(&thread_pool.task_que.cond);

                for(int j = 0; j < thread_pool.thread_num; ++j){
                    pthread_join(thread_pool.pthid[j], NULL);
                }
                
#ifdef _DEBUG
                printf("main thread exit\n");
#endif
                exit(0);
            }// end else if
            else{
                for(int m = 0; m < 10; ++m){
                    if(1 == user_info[m].con_flag && evs[i].data.fd == user_info[m].main_socket_fd){
                        int cur_cli_fd;
                        cur_cli_fd = evs[i].data.fd;

                        ret = recv(cur_cli_fd, &order, 4, 0);
                        if(0 == ret){
                            printf("client dont conn\n");
                            epollDelFd(epfd, cur_cli_fd);
                            close(cur_cli_fd);
                            memset(user_info + m, 0, sizeof(UserInfo));
                            break;
                        }
                        switch(order){
                        case 1:
                            /* 注册请求 */
                            // 接收user_name
                            //ret = recv(cur_cli_fd, &cur_len, 4, 0);                 
                            //if(0 == ret){
                            //    printf("client dont conn\n");
                            //    epollDelFd(epfd, cur_cli_fd);
                            //    close(cur_cli_fd);
                            //    memset(user_info + m, 0, sizeof(UserInfo));
                            //    //goto conn_over;
                            //    break;
                            //}
                            bzero(cur_name, sizeof(cur_name));
                            ret = recv(cur_cli_fd, cur_name, 20, MSG_WAITALL);
                            if(0 == ret){
                                printf("client dont conn\n");
                                epollDelFd(epfd, cur_cli_fd);
                                close(cur_cli_fd);
                                memset(user_info + m, 0, sizeof(UserInfo));
                                //goto conn_over;
                                break;
                            }
                            // 通过名字去数据库中找
                            int have_user;
                            have_user = findUserByName(sql_conn, cur_name);
                            ret = send(cur_cli_fd, &have_user, 4, 0);
                            if(-1 == ret){
                                printf("client dont conn\n");
                                epollDelFd(epfd, cur_cli_fd);
                                close(cur_cli_fd);
                                memset(user_info + m, 0, sizeof(UserInfo));
                                //goto conn_over;
                                break;
                            }
                            
                            if(0 == have_user){
                                // 生成salt值，发送给客户端
                                bzero(cur_salt, sizeof(cur_salt));
                                createSalt(cur_salt, 2);
#ifdef _DEBUG
                                printf("salt:%s\n", cur_salt);
#endif
                                ret = send(cur_cli_fd, cur_salt, 2, 0);
                                if(-1 == ret){
                                    printf("client dont conn\n");
                                    epollDelFd(epfd, cur_cli_fd);
                                    close(cur_cli_fd);
                                    memset(user_info + m, 0, sizeof(UserInfo));
                                    break;
                                }
                                // 接收密文
                                bzero(cur_secret, sizeof(cur_secret));
                                ret = recv(cur_cli_fd, cur_secret, 30, 0);
                                if(0 == ret){
                                    printf("client dont conn\n");
                                    epollDelFd(epfd, cur_cli_fd);
                                    close(cur_cli_fd);
                                    memset(user_info + m, 0, sizeof(UserInfo));
                                    break;
                                }
#ifdef _DEBUG
                                printf("接收了%d字节\n", ret);
                                printf("secret:%s\n",cur_secret);
                                fflush(stdout);
#endif
                                // 生成token
                                bzero(cur_token, sizeof(cur_token));
                                createToken(cur_token, 30);
                                // 将这些内容写入数据库
                                ret = addUser(sql_conn, cur_name, cur_salt, cur_secret, cur_token);
                                char type = 'd';
                                int add_file_ret;
                                add_file_ret = addRootFile(sql_conn, cur_name, cur_name, type, 0, -1);
                                ret = send(cur_cli_fd, &ret, 4, 0);
                                if(-1 == ret){
                                    printf("client dont conn\n");
                                    epollDelFd(epfd, cur_cli_fd);
                                    close(cur_cli_fd);
                                    memset(user_info + m, 0, sizeof(UserInfo));
                                    break;
                                }
                                bzero(cwd, sizeof(cwd));
                                getcwd(cwd, sizeof(cwd));
#ifdef _DEBUG
                                printf("cwd:%s\n", cwd);
#endif
                                ret = chdir("../user_file");
                                ERROR_CHECK(ret, -1, "chdir");
                                ret = mkdir(cur_name, 0777);
                                ERROR_CHECK(ret, -1, "mkdir");
                                chdir(cwd);
                            }else{
                                break;
                            }
                            break;
    
                        case 2:
                            /* 登录请求 */
                            // recv name
                            printf("login\n");
                            bzero(cur_name, sizeof(cur_name));
                            ret = recv(cur_cli_fd, cur_name, 20, 0);
                            if(0 == ret){
                                printf("client dont conn\n");
                                epollDelFd(epfd, cur_cli_fd);
                                close(cur_cli_fd);
                                memset(user_info + m, 0, sizeof(UserInfo));
                                break;
                            }
                            //ret = recv(cur_cli_fd, cur_name, name_len, 0);
                            //if(0 == ret){
                            //    printf("client dont conn\n");
                            //    epollDelFd(epfd, cur_cli_fd);
                            //    close(cur_cli_fd);
                            //    memset(user_info + m, 0, sizeof(UserInfo));
                            //    break;
                            //}
                            //
                            //  数据库中查找
                            ret = findUserByName(sql_conn, cur_name);
                            int find_result;
                            if(-1 == ret){
                                // 该用户存在
                                find_result = 0;
                                ret = send(cur_cli_fd, &find_result, 4, 0);
                                if(-1 == ret){
                                    printf("client dont conn\n");
                                    epollDelFd(epfd, cur_cli_fd);
                                    close(cur_cli_fd);
                                    memset(user_info + m, 0, sizeof(UserInfo));
                                    break;
                                }
                                // 去数据库中读出用户信息并初始化结构体
                                ret = getUserInfo(sql_conn, cur_name, user_info + m);
                                if(0 == ret){
                                    // 读取成功
                                    // send salt;
                                    ret = send(cur_cli_fd, user_info[m].u_salt, 2, 0);
                                    if(-1 == ret){
                                        printf("client dont conn\n");
                                        epollDelFd(epfd, cur_cli_fd);
                                        close(cur_cli_fd);
                                        memset(user_info + m, 0, sizeof(UserInfo));
                                        break;
                                    }

                                    // recv secret;
                                    bzero(cur_secret, sizeof(cur_secret));
                                    ret = recv(cur_cli_fd, cur_secret, 30, MSG_WAITALL);
                                    if(0 == ret){
                                        printf("client dont conn\n");
                                        epollDelFd(epfd, cur_cli_fd);
                                        close(cur_cli_fd);
                                        memset(user_info + m, 0, sizeof(UserInfo));
                                        break;
                                    }
                                    // compare secret
                                    if(strcmp(user_info[m].u_pswd, cur_secret) != 0){
                                        // secret error
                                        int correct = -1;
                                        ret = send(cur_cli_fd, &correct, 4, 0);
                                        if(-1 == ret){
                                            printf("client dont conn\n");
                                            epollDelFd(epfd, cur_cli_fd);
                                            close(cur_cli_fd);
                                            memset(user_info + m, 0, sizeof(UserInfo));
                                            break;
                                        }
                                    }else{
                                        // secret correct
                                        int correct = 0;
                                        ret = send(cur_cli_fd, &correct, 4, 0);
                                        if(-1 == ret){
                                            printf("client dont conn\n");
                                            epollDelFd(epfd, cur_cli_fd);
                                            close(cur_cli_fd);
                                            memset(user_info + m, 0, sizeof(UserInfo));
                                            break;
                                        }
                                        // 把token值发送过去
                                        ret = send(cur_cli_fd, user_info[m].u_token, 30, 0);
                                        if(-1 == ret){
                                            printf("client dont conn\n");
                                            epollDelFd(epfd, cur_cli_fd);
                                            close(cur_cli_fd);
                                            memset(user_info + m, 0, sizeof(UserInfo));
                                            break;
                                        }
                                        // 初始化其他信息
                                        initUserInfo(user_info + m);
                                        // 将信息发送到客户端   path, level, level_dad
                                        // ret = send(cur_cli_fd, user_info[m].u_path, 50, 0);
                                        // if(-1 == ret){
                                        //     printf("client dont conn\n");
                                        //     epollDelFd(epfd, cur_cli_fd);
                                        //     close(cur_cli_fd);
                                        //     memset(user_info + m, 0, sizeof(UserInfo));
                                        //     break;
                                        // }
                                        ret = send(cur_cli_fd, &user_info[m].f_level, 4, 0);
                                        if(-1 == ret){
                                            printf("client dont conn\n");
                                            epollDelFd(epfd, cur_cli_fd);
                                            close(cur_cli_fd);
                                            memset(user_info + m, 0, sizeof(UserInfo));
                                            break;
                                        }
                                        ret = send(cur_cli_fd, &user_info[m].f_level_dad, 4, 0);
                                        if(-1 == ret){
                                            printf("client dont conn\n");
                                            epollDelFd(epfd, cur_cli_fd);
                                            close(cur_cli_fd);
                                            memset(user_info + m, 0, sizeof(UserInfo));
                                            break;
                                        }
                                        // 新加的cur_cli_fd send to client 
                                        ret = send(cur_cli_fd, &user_info[m].main_socket_fd, 4, 0);
                                        if(-1 == ret){
                                            closeConn(epfd, user_info, m);
                                            break;
                                        }
#ifdef _DEBUG
                                        printf("main_socket_fd:%d,%d\n", user_info[m].main_socket_fd, cur_cli_fd);
                                        printf("%s,%s,%s,%s,%s,%d\n", user_info[m].u_name, user_info[m].u_pswd, 
                                               user_info[m].u_token, user_info[m].u_salt, user_info[m].u_path, 
                                               user_info[m].f_level);
                                        printf("%s\n", user_info[m].u_salt);
                                        printf("%s\n", user_info[m].u_path);
#endif
                                    }
                                }else{
                                    // 暂时未做处理(在数据库中读数据失败)
                                    printf("read user_info error\n");
                                    break;
                                }   
                            }else{
                                // 用户不存在
                                find_result = -1;
                                ret = send(cur_cli_fd, &find_result, 4, 0);
                                if(-1 == ret){
                                    printf("client dont conn\n");
                                    epollDelFd(epfd, cur_cli_fd);
                                    close(cur_cli_fd);
                                    memset(user_info + m, 0, sizeof(UserInfo));
                                    break;
                                }
                                break;
                            }
                            break;

                        case 11:
                            // 接收文件夹名，数据库里添加一条记录
                            bzero(second, sizeof(second));                            
                            ret = recv(cur_cli_fd, second, 20, 0);
                            if(0 == ret){
                                closeConn(epfd,user_info, m);
                                break;
                            }
#ifdef _DEBUG
                            printf("dir:%s\n", second);
#endif
                            int find_dir_res;
                            find_dir_res = findDir(sql_conn, user_info[m].u_name, second, 
                                                   user_info[m].f_level + 1);
                            int mkdir_result;
                            if(0 == find_dir_res){
                                addDir(sql_conn, user_info[m].u_name, second, 'd', 
                                       user_info[m].f_level + 1, user_info[m].f_level);
                                // 相应的目录下，创建一个目录
                                bzero(cwd, sizeof(cwd));
                                getcwd(cwd, sizeof(cwd));
                                chdir(user_info[m].u_path);
#ifdef _DEBUG
                                printf("u_path:%s\n", user_info[m].u_path);
#endif
                                mkdir(second, 0777);
                                chdir(cwd);
                                mkdir_result = 0;
                                ret = send(cur_cli_fd, &mkdir_result, 4, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                            }else{
                                mkdir_result = -1;
                                ret = send(cur_cli_fd, &mkdir_result, 4, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                            }
                            break;
                        // cd命令
                        case 4:
#ifdef _DEBUG
                            printf("now:%s,%d,%d\n", user_info[m].u_path, user_info[m].f_level, user_info[m].f_level_dad);
#endif
                            bzero(second, sizeof(second));
                            ret = recv(cur_cli_fd, second, 20, 0);
                            if(0 == ret){
                                closeConn(epfd, user_info, m);
                                break;
                            }
#ifdef _DEBUG
                            printf("cd dir:%s\n", second);
#endif
                            // 把路径分割开放到栈里面
                            char cur_dir[5][20];
                            bzero(cur_dir, sizeof(cur_dir));
                            dirSplit(second, cur_dir);
#ifdef _DEBUG
                            for(int i = 0; i < 5; ++i){
                                if(0 != strcmp(cur_dir[i], ""))
                                    printf("%d:%s\n", i, cur_dir[i]);
                                else
                                    break;
                            }
                            fflush(stdout);
#endif
                            int chdir_res;
                            chdir_res = myChDir(sql_conn, user_info, m, cur_dir);
                            int chdir_result;
                            if(0 == chdir_res){
                                // 改变user_info里面的信息,只有路径信息，其他的已经改了
                                chUserInfo(user_info, m, cur_dir);
                                chdir_result = 0;
                                ret = send(cur_cli_fd, &chdir_result, 4, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                            }else{
                                chdir_result = -1;
                                ret = send(cur_cli_fd, &chdir_result, 4, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                            }
#ifdef _DEBUG
                            printf("now:%s,%d,%d\n", user_info[m].u_path, user_info[m].f_level, user_info[m].f_level_dad);
#endif
                            break;
                        
                        // ls 
                        case 3:
#ifdef _DEBUG
                            printf("userInformation:%s%d\n", user_info[m].u_name, user_info[m].f_level);
#endif
                            int ls_res;
                            char ls_result[100];
                            memset(ls_result, 0, sizeof(ls_result));
                            ls_res = lsFunc(sql_conn, user_info, m, ls_result);
#ifdef _DEBUG
                            printf("ls:%s", ls_result);
#endif
                            if(0 == ls_res){
                                // 查询成功
                                ret = send(cur_cli_fd, &ls_res, 4, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                                ret = send(cur_cli_fd, ls_result, 100, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                            }else if(-1 == ls_res){
                                // 查询失败
                                ret = send(cur_cli_fd, &ls_res, 4, 0);
                                if(-1 == ret){
                                    closeConn(epfd, user_info, m);
                                    break;
                                }
                            }
                            break;

                        // upload上传命令
                        case 5:
                           printf("i will create a new task and insert to taskque\n"); 
                           
                           break;
                        
                        // download 普通下载命令 
                        case 6:
                            break;
                        // downloads 多点下载
                        case 7:
                            break;
                        // ps 查看下载上传进度
                        // 好像这里用不到
                        case 8:
                            break;
                        // del 删除文件
                        case 9:
                            break;

                        // quit退出
                        case 10:
                            break;




                        }            
                    }
                }

            }
        }// endfor

    }// endwhile

}
