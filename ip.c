#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h>

// 获取ODV节点活动网络接口模块
// 默认ODV节点之存在一个可用网络接口

int get_ip_and_inf(char *ipaddr,char *inf) {
    struct ifaddrs *ifAddrStruct;
    char ip[16];
    memset(ip, 0, sizeof(ip));
    void *tmpAddrPtr=NULL;
    getifaddrs(&ifAddrStruct);
    while (ifAddrStruct != NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) {
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
            //printf("%s IP Address:%s\n", ifAddrStruct->ifa_name, ip);
            if (strcmp(ip, "127.0.0.1") != 0) {
                sprintf(ipaddr,ip);
                sprintf(inf,ifAddrStruct->ifa_name);
            }            
        }
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    //free ifaddrs
    freeifaddrs(ifAddrStruct);
    return 0;
}

/*
int main()
{
    char ip[16];
    char iface[IFNAMSIZ];
    char tt[16];
    memset(ip, 0, sizeof(ip));
    memset(tt, 0, sizeof(tt));
    get_ip_and_inf(ip,iface);
    printf("%s\n",ip);
    printf("%s\n",iface);
    strncpy(tt,ip,7);
    printf("%s\n",tt);
    return 0;
}
*/
