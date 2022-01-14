#include "../include/userinfo.h"

/*
* 注册添加用户,写入数据库
* 0:success, -1:fail重名(后面的每个函数都是这样)
*/
int addUser(MYSQL* sql_conn, char* name, char* salt, char* pswd, char* token){


    char query[200]="insert into user (u_name, u_pswd, u_token, u_salt)values(";
    sprintf(query, "%s'%s', '%s', '%s', '%s')",query, name, pswd, token, salt);
    printf("query= %s\n",query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else{
        printf("insert success\n");
    }
    return 0;
}


void initUserInfo(UserInfo* user_info){
    // sprintf(user_info->u_path, "%s%s%s", user_info->u_path, user_info->u_name, "@ubuntu:~/");
    strcat(user_info->u_path, "../user_file/");
    strcat(user_info->u_path, user_info->u_name);
    user_info->f_level = 0;
    user_info->f_level_dad = -1;
    user_info->con_flag = 1;
    user_info->login_flag = 1;

}


void chUserInfo(UserInfo* user_info, int m, char (*dir)[20]){
    
    int len = strlen(user_info[m].u_path);
    int k = len;
    for(int i = 0; i < 5; ++i){
        if(0 == strcmp(dir[i], "")){
            break;
        }else{
            if(0 == strcmp(dir[i], "..")){
                for(; k >= 0; --k){
                    if(user_info[m].u_path[k] != '/'){
                        user_info[m].u_path[k] = 0;
                    }else{
                        user_info[m].u_path[k] = 0;
                        --k;
                        break;
                    }
                }
            }else{
                strcat(user_info[m].u_path, "/");
                strcat(user_info[m].u_path, dir[i]);
            }
        }
    }
#ifdef _DEBUG

    printf("zhixingdaozhele\n");
#endif
    return;

    
}
