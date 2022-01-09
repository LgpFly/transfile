#include "../include/sql.h"
#include "../include/userinfo.h"

int sqlConnect(MYSQL **conn)
{
    char server[]="localhost";
    char user[]="root";
    char password[]="123456";
    char database[]="transfile";//要访问的数据库名称
    *conn=mysql_init(NULL);
    if(!mysql_real_connect(*conn,server,user,password,database,0,NULL,0))
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

void addDir(MYSQL* sql_conn, char* u_name, char* f_name, char type, int f_level, int f_level_dad){
    addRootFile(sql_conn, u_name, f_name, type, f_level, f_level_dad);
}

int findDir(MYSQL* sql_conn, char* u_name, char* f_name, int f_level){
    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[300];
    bzero(query, sizeof(query));
    strcat(query, "select f_id from fileinfo where u_name='");
    strcat(query, u_name);
    strcat(query, "' and f_name='");
    strcat(query, f_name);
    strcat(query, "' and f_level=");
    sprintf(query, "%s%d", query, f_level);
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






