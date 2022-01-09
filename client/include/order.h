#ifndef __ORDER_H__
#include "head.h"

enum Order{
    post = 1, 
    login = 2, 
    ls = 3,
    cd = 4,
    upload = 5, 
    download = 6, 
    downloads = 7,
    ps = 8,
    del = 9, 
    quit = 10,
    mdir = 11
};


void orderSplit(char*, char*, char*);

void print(char*);
#endif

