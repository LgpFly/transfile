#include "../include/transfile.h"
#include "../include/md5.h"

// 接收一个文件
// int recvFile(int client_fd){
//     int ret;
//     // recv fileName
//     int file_name_len;
//     char file_name[50] = {0};
//     ret = recv(client_fd, &file_name_len, 4, MSG_WAITALL);
//     if(0 == ret){
// #ifdef _DEBUG
//         printf("server fly\n");
// #endif
//         return -1;
//     }
//     ret = recv(client_fd, file_name, file_name_len, MSG_WAITALL);
//     if(0 == ret){
// #ifdef _DEBUG
//         printf("server fly\n");
// #endif
//         return -1;
//     }
// 
//     // recv size
//     int file_size_len;
//     off_t file_size;
//     ret = recv(client_fd, &file_size_len, 4, MSG_WAITALL);
//     if(0 == ret){
// #ifdef _DEBUG
//         printf("server fly\n");
// #endif
//         return -1;
//     }
//     
//     ret = recv(client_fd, &file_size, file_size_len, MSG_WAITALL);
//     if(0 == ret){
// #ifdef _DEBUG
//         printf("server fly\n");
// #endif
//         return -1;
//     }
//     off_t slice = file_size / 100;
// 
// 
//     // open file
//     int file_fd;
//     file_fd = open(file_name, O_RDWR|O_CREAT, 0666);
//     ERROR_CHECK(file_fd, -1, "open file");
// 
//     // recv content & write file
//     int recv_num = 0;
//     int one_recv_num = 0;
//     int last_recv_num = 0;
//     char buf[1020];
//     int buf_len;
//     while(recv_num < file_size){
//         memset(buf, 0, sizeof(buf));
//         ret = recv(client_fd, &buf_len, 4, 0);
//         if(0 == ret){
// #ifdef _DEBUG
//             printf("server fly\n");
// #endif
//             close(file_fd);
//             return -1;
//         }
//         one_recv_num = recv(client_fd, buf, buf_len, MSG_WAITALL);
//         if(0 == ret){
// #ifdef _DEBUG
//             printf("server fly\n");
// #endif
//             close(file_fd);
//             return -1;
//         }
//         recv_num += one_recv_num;
//         ret = write(file_fd, buf, buf_len);
//         ERROR_CHECK(ret, -1, "write(content)");
//         
//         // 打印进度条
//         if((recv_num - last_recv_num)  > slice){
//             last_recv_num = recv_num;
//             printf("download:%5.2f%%\r", (float)recv_num / file_size * 100);
//             fflush(stdout);
//         }
//     }
//     printf("download:100.00%%\n");
//     fflush(stdout);
//     
//     close(file_fd);
//     return 0;
// }


int getFileInfo(char* f_name, UpFileInfo* up_file_info){
    
    int ret;
    char md5[33];
    strcpy(up_file_info->f_name, f_name);
    struct stat statbuf;
    memset(&statbuf, 0, sizeof(statbuf));
    int fd = open(f_name, O_RDWR);
    if(-1 == fd){
        printf("open file error\n");
        return -1;
    }
    ret = fstat(fd, &statbuf);
    up_file_info->f_size = statbuf.st_size;
    close(fd);
    bzero(md5, sizeof(md5));
    ret = computeFileMd5(f_name, md5);
    if(-1 == ret){
        printf("get md5 error\n");
        return -1;
    }
    strcpy(up_file_info->f_md5, md5);
#ifdef _DEBUG
    printf("f_size:%ld,md5:%s", up_file_info->f_size, up_file_info->f_md5);
#endif
    return 0;

}


int sendFile(int fd, char* name, long size){
    
    int ret;
    Train train;
    long already_send = 0;
    int one_send = 0;
    int file_fd = open(name, O_RDWR);
    while(already_send < size)
    {
        bzero(&train, sizeof(train));
        one_send = read(file_fd, train.buf, sizeof(train.buf));
        train.data_len = one_send;
        ret = send(fd, &train, 4 + train.data_len, 0);
        if(-1 == ret){
            printf("server fly\n");
            close(fd);
            close(file_fd);
            return -1;
        }
        already_send += one_send;
    }
    return 0;

}


int recvsFile(int cli_fd, char* f_name, long f_size, long f_seek){
    
    int ret;
    int file_fd = open(f_name, O_CREAT|O_RDWR, 0666);
    lseek(file_fd, f_seek, 0);
    long already_recv = 0;
    int one_recv;
    char buf[1020];
    while(already_recv < f_size){
        ret = recv(cli_fd, &one_recv, 4, MSG_WAITALL);
        if(0 == ret){
            printf("server fly\n");
            close(file_fd);
            close(cli_fd);
            return -1;
        }
        bzero(buf, sizeof(buf));
        ret = recv(cli_fd, buf, one_recv, MSG_WAITALL);
        if(0 == ret){
            printf("server fly\n");
            close(file_fd);
            close(cli_fd);
            return -1;
        }
        ret = write(file_fd, buf, one_recv);

    }
    close(file_fd);
    return 0;


}


int recvFile(int cli_fd, char* f_name, long f_size){
    
    int ret;

    int file_fd = open(f_name, O_CREAT|O_RDWR, 0666);
    long already_recv = 0;
    int one_recv;
    char buf[1020];
    while(already_recv < f_size){
        ret = recv(cli_fd, &one_recv, 4, MSG_WAITALL);
        if(0 == ret){
            printf("server fly\n");
            close(file_fd);
            close(cli_fd);
            return -1;
        }
        bzero(buf, sizeof(buf));
        ret = recv(cli_fd, buf, one_recv, MSG_WAITALL);
        if(0 == ret){
            printf("server fly\n");
            close(file_fd);
            close(cli_fd);
            return -1;
        }
        ret = write(file_fd, buf, one_recv);

    }
    close(file_fd);
    return 0;


}

