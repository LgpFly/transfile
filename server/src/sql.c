#include "../include/sql.h"
#include "../include/userinfo.h"
#include "../include/head.h"


int sqlConnect(MYSQL **conn)
{
    char server[]="localhost";
    char user[]="root";
    char password[]="123456";
    char database[]="transfile";//要访问的数据库名称
    *conn=mysql_init(NULL);
    if(!mysql_real_connect(*conn,server,user,password,database,0,NULL,CLIENT_MULTI_RESULTS))
    {
#ifdef _DEBUG
        printf("Error connecting to database:%s\n",mysql_error(*conn));
#endif
        return -1;
    }else{
#ifdef _DEBUG
        printf("Connected sql success\n");
#endif
    }
    return 0;
}


void createSalt(char *str, int len){
    str[len] = 0;
    int i,flag;
    srand(time(NULL));
    for(i = 0; i < len; ++i)
    {
        flag = rand()%3;
        switch(flag)
        {
        case 0:
            str[i] = rand()%26 + 'a';
            break;
        case 1:
            str[i] = rand()%26 + 'A';
            break;
        case 2:
            str[i] = rand()%10 + '0';
            break;
        }
    }
    // *str='$';
    return;
}

void createToken(char *str, int len){
    str[len] = 0;
    int i,flag;
    srand(time(NULL));
    for(i = 0; i < len - 1; ++i)
    {
        flag = rand()%3;
        switch(flag)
        {
        case 0:
            str[i] = rand()%26 + 'a';
            break;
        case 1:
            str[i] = rand()%26 + 'A';
            break;
        case 2:
            str[i] = rand()%10 + '0';
            break;
        }
    }
    *str='&';
    return;
}

int findUserByName(MYSQL* sql_conn, char* name){
    
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300]="select u_name from user where u_name='";
    sprintf(query, "%s%s%s", query, name, "'");
    puts(query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else{
        res = mysql_use_result(sql_conn);
        if(res)
        {
            if((row = mysql_fetch_row(res)) != NULL)
            {
                if(strcmp(row[0], name) == 0){
                    mysql_free_result(res);
                    return -1;
                }
            }
        }else{
            printf("查询出错\n");
            return -1;
        }
        mysql_free_result(res);
    }

    return 0;
}

int getUserInfo(MYSQL* sql_conn, char* name, UserInfo* user_info){

    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300]="select u_pswd, u_token, u_salt from user where u_name='";
    sprintf(query, "%s%s%s", query, name, "'");
    puts(query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else{
        res = mysql_use_result(sql_conn);
        if(res)
        {
            if((row = mysql_fetch_row(res)) != NULL)
            {
                strcpy(user_info->u_name, name);
                strcpy(user_info->u_pswd, row[0]);
                strcpy(user_info->u_token, row[1]);
                strcpy(user_info->u_salt, row[2]);
            }else{
                printf("查询出错\n");
                mysql_free_result(res);
                return -1;
            }
            mysql_free_result(res);
        }
    }
    return 0; 

}

int addRootFile(MYSQL* sql_conn, char* u_name, char* f_name, char type, int f_level, int f_level_dad){
    
    char query[200]="insert into fileinfo (u_name, f_name, f_type, f_level, f_level_father)values(";                                                                                     
    sprintf(query, "%s'%s', '%s', '%c', '%d', %d)",query, u_name, f_name, type, f_level, f_level_dad);
    printf("query= %s\n",query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
#ifdef _DEBUG
        printf("Error making query:%s\n",mysql_error(sql_conn));
#endif
        return -1;
    }else{
#ifdef _DEBUG
        printf("insert success\n");
#endif
        return 0;
    }
}

void addDir(MYSQL* sql_conn, char* u_name, char* f_name, char type, int f_level, int f_level_dad, char* dir_dad){
    
    char query[200]="insert into fileinfo (u_name, f_name, f_type, f_level, f_level_father, dir_dad)values(";                                                                                     
    sprintf(query, "%s'%s', '%s', '%c', '%d', %d, '%s')",query, u_name, f_name, type, f_level, f_level_dad, dir_dad);
    printf("query= %s\n",query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
#ifdef _DEBUG
        printf("Error making query:%s\n",mysql_error(sql_conn));
#endif
    }else{
#ifdef _DEBUG
        printf("insert success\n");
#endif
    }

}

int findDir(MYSQL* sql_conn, char* u_name, char* f_name, int f_level, char* dir_dad){
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300];
    bzero(query, sizeof(query));
    strcat(query, "select f_id from fileinfo where u_name='");
    strcat(query, u_name);
    strcat(query, "' and f_name='");
    strcat(query, f_name);
    strcat(query, "' and f_type='d'");
    strcat(query, "and f_level=");
    sprintf(query, "%s%d", query, f_level);
    strcat(query, " and dir_dad='");
    strcat(query, dir_dad);
    strcat(query, "'");
    puts(query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else{
        res = mysql_use_result(sql_conn);
        if(res)
        {
            if((row = mysql_fetch_row(res)) != NULL)
            {
                mysql_free_result(res);
                return -1;
            }
        }else{
            printf("查询出错\n");
            return -1;
        }
        mysql_free_result(res);
    }

    return 0;

}

int myChDir(MYSQL* sql_conn, UserInfo* user_info, int m, char (*dir)[20]){
    
#ifdef _DEBUG
    printf("now:%d,%d,%s\n", user_info[m].f_level, user_info[m].f_level_dad, user_info[m].dir_dad);
#endif
    
    MYSQL_RES *res;
    MYSQL_ROW row;
    int level = user_info[m].f_level;
    int level_dad = user_info[m].f_level_dad;
    int old_level = level;
    int old_level_dad = level_dad;
    char old_dir_dad[20];
    bzero(old_dir_dad, sizeof(old_dir_dad));
    strcpy(old_dir_dad, user_info[m].dir_dad);
    char name[20];
    bzero(name, sizeof(name));
    strcpy(name, user_info[m].u_name);
    char query[300];
    int t;
    for(int i = 0; i < 5; ++i){
        if(0 != strcmp(dir[i], "")){
            if(0 == strcmp(dir[i], "..")){
                if(-1 == level_dad){
                    return -1;
                }else{
                    bzero(query, sizeof(query));
                    strcat(query, "select dir_dad from fileinfo where u_name = '");
                    sprintf(query, "%s%s%s%s%s%s%s", query, name, "'", " and f_name='", user_info[m].dir_dad, "'", 
                            " and f_type = 'd'");
                    puts(query);
                    t = mysql_query(sql_conn, query);
                    if(t){
                        printf("error\n");
                        return -1;
                    }else{
                        res = mysql_use_result(sql_conn);
                        if(res){
                            if((row = mysql_fetch_row(res)) != NULL){
                                level = level_dad;
                                level_dad--;
                                strcpy(user_info[m].dir_dad, row[0]);
                                mysql_free_result(res);
                            }else{
                                
                            }
                        }else{
                            printf("查询出错\n");
                            mysql_free_result(res);
                            return -1;
                        }
                    }
                }
            }else{
                bzero(query, sizeof(query));
                strcat(query, "select f_level from fileinfo where u_name='");
                sprintf(query, "%s%s%s%s%d%s%s%s%s%s%s%s", query, name, "'", " and f_level_father=", level, " and f_type='d'", 
                        " and f_name='", dir[i], "'", " and dir_dad='", user_info[m].dir_dad, "'");
                puts(query);
                t = mysql_query(sql_conn, query);
                if(t){
                    printf("error\n");
                    return -1;
                }else{
                    res = mysql_use_result(sql_conn);
                    if(res){
                        if((row = mysql_fetch_row(res)) != NULL){
                            level_dad = level;
                            level = atoi(row[0]);
                            bzero(user_info[m].dir_dad, sizeof(user_info[m].dir_dad));
                            strcpy(user_info[m].dir_dad, dir[i]);
                            mysql_free_result(res);
                        }else{
#ifdef _DEBUG
                            printf("no this dir\n");

#endif
                            user_info[m].f_level = old_level;
                            user_info[m].f_level_dad = old_level_dad;
                            strcpy(user_info[m].dir_dad, old_dir_dad);
                            return -1;
                        }

                    }else{
                        printf("error\n");
                        mysql_free_result(res);
                        return -1;
                    }
                }
            }
        }else{
            user_info[m].f_level = level;
            user_info[m].f_level_dad = level_dad;
            break;
        }
    }
#ifdef _DEBUG
    printf("now:%d,%d,%s\n", user_info[m].f_level, user_info[m].f_level_dad, user_info[m].dir_dad);
#endif
    return 0;
}

int lsFunc(MYSQL* sql_conn, UserInfo* user_info, int m, char* result){

    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300]="select f_name, f_type, f_size from fileinfo where u_name = '";
    sprintf(query, "%s%s%s%s%d%s%s%s", query, user_info[m].u_name, "'", 
            " and f_level = ", user_info[m].f_level + 1, " and dir_dad='", user_info[m].dir_dad, "'");
#ifdef _DEBUG
    puts(query);
#endif
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else
    {
        res=mysql_use_result(sql_conn);
        if(res)
        {
            while((row=mysql_fetch_row(res))!=NULL)
            {
                // sprintf(result, "%s%s%s%s%s%lld\n", result, row[0], "--", row[1], "--", atoll(row[2]));
                if(NULL == row[2])
                    sprintf(result, "%s%-10s%s\n", result, row[0], row[1]);
                else
                    sprintf(result, "%s%s%s%s%s%lld\n", result, row[0], "--", row[1], "--", atoll(row[2]));

            }
        }else{
#ifdef _DEBUG
            printf("res error\n");
#endif
            return -1;
        }
        mysql_free_result(res);
    }
    return 0;
}

// f_level其实是他的dad级别，他自己的级别应该是f_level+1
int addFile(MYSQL* sql_conn, char* u_name, char* f_name, long f_size, char* md5, int f_level, char *dir_dad){
    
    char type = 'f';
    char query[300]="insert into fileinfo (u_name, f_name, f_size, f_type, f_level, f_level_father, md5, dir_dad)values(";                                                                                     
    sprintf(query, "%s'%s', '%s', %ld, '%c', %d, %d, '%s', '%s')",query, u_name, f_name, f_size, type, f_level + 1, f_level, md5, dir_dad);
    printf("query= %s\n",query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
#ifdef _DEBUG
        printf("Error making query:%s\n",mysql_error(sql_conn));
#endif
        return -1;
    }else{
#ifdef _DEBUG
        printf("insert success\n");
#endif
        return 0;
    }
    
}


long findFile(MYSQL* sql_conn, char* u_name, char* f_name, int f_level, char* dir_dad){
    MYSQL_RES *res;
    MYSQL_ROW row;
    long file_size;
    char query[300];
    bzero(query, sizeof(query));
    strcat(query, "select f_size from fileinfo where u_name='");
    strcat(query, u_name);
    strcat(query, "' and f_name='");
    strcat(query, f_name);
    strcat(query, "' and f_type='f'");
    strcat(query, " and f_level=");
    sprintf(query, "%s%d%s%s%s", query, f_level, " and dir_dad='", dir_dad, "'");
    
    puts(query);
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else{
        res = mysql_use_result(sql_conn);
        if(res)
        {
            if((row = mysql_fetch_row(res)) != NULL)
            {
                // 这里返回一个局部变量，当然会出错的了
                file_size = atoll(row[0]);
                // printf("%ld\n", file_size);
                mysql_free_result(res);
                return file_size;
            }
        }else{
            printf("查询出错\n");
            mysql_free_result(res);
            return -1;
        }
        mysql_free_result(res);
    }

}


int delDir(char* u_name, int f_level, char* f_name){
    
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300]="select f_name, f_type from fileinfo where u_name = '";
    sprintf(query, "%s%s%s%s%d%s%s%s", query, u_name, "'", 
            " and f_level = ", f_level + 1, " and dir_dad='", f_name, "'");
#ifdef _DEBUG
    puts(query);
#endif
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else
    {
        res = mysql_use_result(sql_conn);
        if(res)
        {
            row = mysql_fetch_row(res);
            if(row  == NULL ){
                // 删除自己本身
                mysql_free_result(res);
                char query1[300] = "delete from fileinfo where u_name = '";
                sprintf(query1, "%s%s%s%s%d%s%s%s", query1, u_name, "'", " and f_level = ", 
                        f_level, " and f_name = '", f_name, "'");
                puts(query1);
                mysql_query(sql_conn, query1);
                // // 释放空间
                return 0;
            }else{
               // if(0 == strcmp(row[1], "f")){
               //     // delFile(u_name, f_level, row[0], f_name);
               // }
               // else{
               //     // mysql_free_result(res);
               //     delDir(u_name, f_level + 1, row[0]);
               //     char query1[300] = "delete from fileinfo where u_name = '";
               //     sprintf(query1, "%s%s%s%s%d%s%s%s", query1, u_name, "'", " and f_level = ", 
               //             f_level, " and f_name = '", f_name, "'");
               //     puts(query1);
               //     mysql_query(sql_conn, query1);
               //     delDir(u_name, f_level + 1, row[0]);
               // }
               // while((row = mysql_fetch_row(res)) != NULL)
               // {

               //     if(0 == strcmp(row[1], "f")){
               //         // delFile(u_name, f_level, row[0], f_name);
               //     }
               //     else{
               //         // mysql_free_result(res);
               //         delDir(u_name, f_level + 1, row[0]);
               //         char query1[300] = "delete from fileinfo where u_name = '";
               //         sprintf(query1, "%s%s%s%s%d%s%s%s", query1, u_name, "'", " and f_level = ", 
               //                 f_level, " and f_name = '", f_name, "'");
               //         puts(query1);
               //         mysql_query(sql_conn, query1);
               //         delDir(u_name, f_level + 1, row[0]);
               //     }

               // }
               mysql_free_result(res);
               return -1;
            }
        }else{
#ifdef _DEBUG
            printf("res error\n");
#endif
            mysql_free_result(res);
            return -1;
        }
        // mysql_free_result(res);
    }
}





int delFile(char* u_name, int f_level, char* f_name, char* dir_dad){

    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300]="select f_type from fileinfo where u_name = '";
    sprintf(query, "%s%s%s%s%d%s%s%s%s%s%s", query, u_name, "'", 
            " and f_level = ", f_level + 1, " and dir_dad='", dir_dad, "'", 
            " and f_name = '", f_name, "'");
#ifdef _DEBUG
    puts(query);
#endif
    int t;
    t=mysql_query(sql_conn,query);
    if(t)
    {
        printf("Error making query:%s\n",mysql_error(sql_conn));
        return -1;
    }else
    {
        res=mysql_use_result(sql_conn);
        if(res)
        {
            if((row=mysql_fetch_row(res))!=NULL)
            {
                if(0 == strcmp(row[0], "f")){
                    
                    mysql_free_result(res);
                    char query1[300]="delete from fileinfo where u_name = '";
                    sprintf(query1, "%s%s%s%s%d%s%s%s%s%s%s", query1, u_name, "'", 
                            " and f_level = ", f_level + 1, " and dir_dad='", dir_dad, "'", 
                            " and f_name = '", f_name, "'");
                    puts(query1);
                    int d = mysql_query(sql_conn, query1);
                    if(d){
#ifdef _DEBUG
                        printf("del error\n");
#endif
                        return -1;
                    }else{
                        printf("del success\n");
                        return 0;
                    }
                }else{
                    mysql_free_result(res);
                    int del_dir = delDir(u_name, f_level + 1, f_name);
                    return del_dir;
                }
            }
        }else{
#ifdef _DEBUG
            printf("res error\n");
#endif
            return -1;
        }
        mysql_free_result(res);
    }
}



