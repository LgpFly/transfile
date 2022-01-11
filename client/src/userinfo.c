#include "../include/userinfo.h"

void initUserInfo(UserInfo* user_info, char* name, char* token, int level, int level_dad){

    strcpy(user_info->u_name, name);
    strcpy(user_info->u_token, token);
    bzero(user_info->u_path, sizeof(user_info->u_path));
    strcat(user_info->u_path, user_info->u_name);
    strcat(user_info->u_path, "@ubuntu:~/");
    // strcat(user_info->u_path, "$");
    user_info->f_level = level;
    user_info->f_level_dad = level_dad;
    return;
}

void chPath(UserInfo* user_info, char (*res)[20]){

    int len = strlen(user_info->u_path);
    int k = len;
    for(int i = 0; i < 5; ++i){

        if(0 == strcmp(res[i], "")){
            break;
        }else if(0 == strcmp(res[i], "..")){
            for(;k >=0; --k){
                if(user_info->u_path[k] != '/'){
                    user_info->u_path[k] = 0;
                }else{
                    user_info->u_path[k] = 0;
                    k--;
                    break;
                }
            }
        }else{
            int len = strlen(user_info->u_path);
            if(user_info->u_path[len - 1] == '/')
                strcat(user_info->u_path, res[i]);
            else{
                strcat(user_info->u_path, "/");
                strcat(user_info->u_path, res[i]);
            }


        }
    }
}
