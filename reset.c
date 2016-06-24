#include <stdio.h>
int GetProfileString(char *Profile, char *AppName, char *KeyName, char *KeyVal);

int main(int argc, char** argv){
    static char ip[16];
    static char inf[32];
    static char bridge[32];
    static char gw[16];
    GetProfileString("./back.conf","backup","ip",ip);
    GetProfileString("./back.conf","backup","inf",inf);
    GetProfileString("./back.conf","backup","bridge",bridge);
    GetProfileString("./back.conf","backup","gw",gw);
    
    /* --- 恢复环境配置 --- */
    
    // 停止网桥
    char cmd1[100];
    sprintf(cmd1,"/sbin/ifconfig %s down",bridge);
    printf("停止网桥[%s]...\n",bridge);
    system(cmd1);
    
    // 删除网桥
    char cmd2[100];
    sprintf(cmd2,"/usr/sbin/brctl delbr %s",bridge);
    printf("删除网桥[%s]...\n",bridge);
    system(cmd2);
    
    // IP地址注入
    char cmd3[100];
    sprintf(cmd3,"/sbin/ifconfig %s %s",inf,ip);
    printf("将IP地址[%s]注入网络接口[%s]\n",ip,inf);
    system(cmd3);
    
    // 配置默认路由
    char cmd4[100];
    sprintf(cmd4,"/sbin/route add -net default gw %s",gw);
    printf("配置默认网关[%s]\n",gw);
    system(cmd4);   
}
