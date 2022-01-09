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
    strcat(path, "$ ");
    printf("%s", path);
    fflush(stdout);
}
