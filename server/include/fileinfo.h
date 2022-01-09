#ifndef __FILEINFO_H__
#define __FILEINFO_H__

#include "head.h"

typedef struct{
    char u_name[20];
    char f_name[20];
    long f_size;
    char f_type;
    int f_level;
    int f_level_dad;
    int f_flag;
}FileInfo, pFileInfo;

/*
 * 创建根目录 
*/
int addFile(MYSQL*, FileInfo*);


#endif
