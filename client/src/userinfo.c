#include "../include/userinfo.h"

void initUserInfo(UserInfo* user_info, char* name, char* token, int level, int level_dad){

    strcpy(user_info->u_name, name);
    strcpy(user_info->u_token, token);
    bzero(user_info->u_path, sizeof(user_info->u_path));
    strcat(user_info->u_path, user_info->u_name);
    strcat(user_info->u_path, "@ubuntu:~/");
    user_info->f_level = level;
    user_info->f_level_dad = level_dad;
    return;
}
