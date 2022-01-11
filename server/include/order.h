#ifndef __ORDER_H__
#define __ORDER_H__

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

void dirSplit(char* dir, char (*res)[20]);


#endif
