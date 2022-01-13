#include "../include/transfile.h"
#include "../include/head.h"
// #define FILE file
// int sendFile(int client_fd){
//     int ret;
//     int read_num;
//     // 先定义小火车的数据结构
//     Train train;
//     bzero(&train, sizeof(train));
//    
//     // 先发送文件名
//     train.data_len = 4;
//     strcpy(train.buf, "file");
//     ret = send(client_fd, &train, 4 + train.data_len, 0);
//     if(-1 == ret){
// #ifdef _DEBUG
//         printf("client fly\n");
// #endif
//         close(client_fd);
//         return -1;
//     }
//     //ERROR_CHECK(ret, -1, "send(name)");
//     // 发送文件大小
//     int file_fd = open("file", O_RDWR);
//     struct stat file_stat;
//     ret = fstat(file_fd, &file_stat);
//     bzero(&train, sizeof(train));
//     train.data_len = sizeof(file_stat.st_size);
//     memcpy(train.buf, &file_stat.st_size, sizeof(file_stat.st_size));
//     ret = send(client_fd, &train, 4 + train.data_len, 0);
//     if(-1 == ret){
// #ifdef _DEBUG
//         printf("client fly\n");
// #endif
//         close(file_fd);
//         close(client_fd);
//         return -1;
//     }
//     //ERROR_CHECK(ret, -1, "send(file_size)");
// 
//     // 发送文件内容
//     off_t file_total_len = file_stat.st_size;
//     off_t already_send = 0;
//     while(already_send < file_total_len){
//         bzero(&train, sizeof(train));
//         read_num = read(file_fd, train.buf, sizeof(train.buf));
//         ERROR_CHECK(read_num, -1, "read");
//         train.data_len = read_num;
//         ret = send(client_fd, &train, 4 + train.data_len, 0);
//         if(-1 == ret){
// #ifdef _DEBUG
//             printf("client fly\n");
// #endif
//             close(file_fd);
//             close(client_fd);
//             return -1;
//     }
//         //ERROR_CHECK(ret, -1, "send(file_size)");
//         already_send += read_num;
//     }
//     close(file_fd);
//     close(client_fd);
//     return 0;
// }

int sendFile(int cli_fd, char *path, char* f_name, long f_size){
    
    int ret;
    Train train;
    char cwd[50];
    bzero(cwd, sizeof(cwd));
    getcwd(cwd, sizeof(cwd));
#ifdef _DEBUG
    printf("now:%s,then:%s", cwd, path);
#endif

    chdir(path);
    int file_fd = open(f_name, O_RDWR);
    if(-1 == file_fd){
#ifdef _DEBUG
        printf("open file error\n");
#endif
        return -1;
    }

    int one_send = 0;
    int already_send = 0;
    while(already_send < f_size){
        bzero(&train, sizeof(train));
        one_send = read(file_fd, train.buf, sizeof(train.buf));
        train.data_len = one_send;
        ret = send(cli_fd, &train, 4 + train.data_len, 0);
        if(-1 == ret){
#ifdef _DEBUG
            printf("send file error(client fly)\n");
#endif
            close(cli_fd);
            return -1;
        }

        already_send += one_send;
    }

    close(file_fd);
    chdir(cwd);

    return 0;
}



