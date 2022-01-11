#include "../include/order.h"
#include "../include/head.h"


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
