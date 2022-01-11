#include "../include/order.h"

void orderSplit(char* order, char* first, char* second){

    int len = strlen(order);
    int i = 0;
    while(i < len && order[i] != ' '){
        ++i;
    }
    if(i == len){
        strcpy(first, order);
    }else{
        strncpy(first, order, i);
        int k = i;
        while(order[k] == ' '){
            k++;
        }
        for(int j = 0; k < len; ++k, ++j){
            second[j] = order[k];
        }
    }

}

void print(char* path){
    // strcat(path, "$ ");
    printf("%s", path);
    printf("%s", "$ ");
    fflush(stdout);
}


void dirSplit(char* dir, char (*res)[20]){
    char *p_flag = strchr(dir, '/');
    int len = strlen(dir);

    if(NULL == p_flag){
        strcpy(res[0], dir);
    }else{
        int count = 0;
        int start = 0;
        for(int i = 0; i < len; ++i){
            if(dir[i] != '/'){
                res[count][start] = dir[i];
                start++;
            }else{
                count++;
                start = 0;
            }
        }
    }
}

