#include "../include/pool.h"
#include "../include/transfile.h"
#include "../include/sql.h"


extern MYSQL* sql_conn;

// 线程池初始化 线程池，线程数量， 队列capacity
int threadPoolInit(pThreadPool p_thread_pool, int thread_num, int task_que_capacity){
    // 结构体使用之前先memset一下
    bzero(p_thread_pool, sizeof(ThreadPool));
    //taskQueInit(&(p_thread_pool->task_que), task_que_capacity);
    taskQueInit(&p_thread_pool->task_que, task_que_capacity);
    p_thread_pool->thread_num = thread_num;
    p_thread_pool->pthid = (pthread_t*)calloc(thread_num, sizeof(pthread_t));
    p_thread_pool->start_flag = 0;
    return 0;
}

// 线程函数
void* threadFunc(void* p_arg){
    int ret;
    pTaskQue p_task_que = (pTaskQue)p_arg;
    pQueNode p_one_node = NULL;
    while(1){
        pthread_mutex_lock(&(p_task_que->mutex));
        if(0 == p_task_que->size){
#ifdef _DEBUG
            printf("wait\n");
#endif
            pthread_cond_wait(&(p_task_que->cond), &(p_task_que->mutex));
#ifdef _DEBUG
            printf("wake up\n");
#endif
        }
        // 取一个任务
        ret = taskQueGet(p_task_que, &p_one_node);
        pthread_mutex_unlock(&(p_task_que->mutex));
        
        if(0 == ret){
            // 再写代码在这里开始写
            if(1 == p_one_node->up_or_down){
                // upload in here
                // recv file and put the correct path then 变回到原来的路径下
                char cwd[50];
                bzero(cwd, sizeof(cwd));
                getcwd(cwd, sizeof(cwd));
                chdir(p_one_node->u_path);
                int file_fd = open(p_one_node->f_name, O_CREAT|O_RDWR, 0666);
                long recv_num = 0;
                int one_recv = 0;
                char buf[1020];
                int buf_len;
                int client_fd = p_one_node->client_fd;
                while(recv_num < p_one_node->f_size){
                    ret = recv(client_fd, &buf_len, 4, 0);
                    if(0 == ret){
                        printf("client dont conn\n");
                        close(client_fd);
                    }
                    bzero(buf, sizeof(buf));
                    one_recv = recv(client_fd, buf, buf_len, MSG_WAITALL);
                    if(0 == one_recv){
                        printf("client dont conn\n");
                        close(client_fd);
                    }
                    recv_num += one_recv;
                    write(file_fd, buf, buf_len);

                }
                chdir(cwd);
                // write info to database
                addFile(sql_conn, p_one_node->u_name, p_one_node->f_name, p_one_node->f_size, p_one_node->f_md5, p_one_node->f_level); 
                
            }

            // 正常是传文件
            // ret = sendFile(p_one_node->client_fd);
            // if(ret == -1){
            // }
        }
        free(p_one_node);
        p_one_node = NULL;
        
        if(1 == p_task_que->exit_flag){
#ifdef _DEBUG
            printf("thread exit\n");
#endif
            pthread_exit(NULL);
        }
    }
}


// 线程池启动
int threadPoolStart(pThreadPool p_thread_pool){
    if(0 == p_thread_pool->start_flag){
        for(int i = 0; i < p_thread_pool->thread_num; i++){
            pthread_create((p_thread_pool->pthid) + i, NULL, threadFunc, &(p_thread_pool->task_que));
        }
        p_thread_pool->start_flag = 1;
    }
    return 0;
}


