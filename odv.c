#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netinet/if_ether.h>
#include <signal.h> 

int tun0, s;
int GetProfileString(char *Profile, char *AppName, char *KeyName, char *KeyVal);
int get_ip_and_inf(char *ipaddr,char *inf);

int tun_create(char *dev, int flags)
{
    struct ifreq ifr;
    int fd, err;

    assert(dev != NULL);

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
        return fd;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags |= flags;
    if (*dev != '\0')
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        close(fd);
        return err;
    }
    strcpy(dev, ifr.ifr_name);

    return fd;
}

int init_tun(char* tun)
{
        char tun_name[IFNAMSIZ];

        printf("创建隧道设备中......\n");
        strcpy(tun_name, tun);
        tun0 = tun_create(tun_name, IFF_TAP | IFF_NO_PI);
        if (tun0 < 0) {
                perror("tun_create");
                return -1;
        }
        printf("TAP name is %s\n", tun_name);

        return 0;
}

void* receiver(void* arg)
{
	char buf[2048];
	while(1)
	{
		printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		int n = recv(s, buf, sizeof(buf), 0);
		printf("[[[[[[%d]]]]]",n);
		printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		if(n < 0)
		{
			perror("recv socket:");
			return 0;
		}
		printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		printf("\n**********************recv %d", n);
		int ret = write(tun0, buf, n);
		if(ret < 0)
		{
			perror("write tap0:");
			return 0;
		}
		//printf("write %d\n", ret);
	}
	return 0;
}

void sigroutine(int dunno) { /* 信号处理例程，其中dunno将会得到信号的值 */ 
    /*
    *  处理 'ctrl - c'  'ctrl - \' HUP
    */
    switch (dunno) { 
        case 1: 
            printf("Get a signal -- SIGHUP \n"); // HUP(关闭命令行窗口)
            system("./reset"); 
            exit(0);
        break; 
        case 2: 
            printf("Get a signal -- SIGINT \n");  // "ctrl - c"
            system("./reset");
            exit(0);
        break; 
        case 3: 
            printf("Get a signal -- SIGQUIT \n");  // "ctrl - \"
            system("./reset");
            exit(0);
        break; 
    } 
    return; 
} 

int main(int argc, char** argv)
{
	struct sockaddr_in local;
	struct sockaddr_in addr;
	pthread_t h;

    /* ------ 注册信号处理方法 ------ */
    printf("ODV软件启动，PID[%d]\n ",getpid()); 
    signal(SIGHUP, sigroutine); //* 下面设置三个信号的处理方法 
    signal(SIGINT, sigroutine); 
    signal(SIGQUIT, sigroutine);     
    /* ------ 注册完毕 ------ */

	/* ------ 读取配置文件 ------ */
	static char lip[16];
	static char rip[16];
	static char tun_name[IFNAMSIZ];
	static char br_name[IFNAMSIZ];
	static char _vmin[10],_vmax[10];
	static int vmin,vmax;
	//GetProfileString("./conf","server","local",lip);
	//不从配置文件读取本地ip后需要在socket绑定时绑定动态获取到的ip
	GetProfileString("./conf","server","remote",rip);
	GetProfileString("./conf","server","tunnel",tun_name);
	GetProfileString("./conf","server","bridge",br_name);
	GetProfileString("./conf","vlan","vmin",_vmin);
	GetProfileString("./conf","vlan","vmax",_vmax);
	/* ------ 读取完毕 ------ */
	//printf("本机跨域通信用IP地址:[%s]\n",ip);
	//printf("对端跨域通信用IP地址:[%s]\n",rip);

    /* ------ 初始化运行环境开始 ------ */
    // 创建备份文件可用于软件异常退出时恢复ODV机器环境
    FILE *fp;
    fp = fopen("back.conf","w+"); 
    fprintf(fp,"[backup]\n");
    
    printf("初始化ODV运行环境.....\n");
    
    // 获取活动接口
    char ip[16];
    memset(ip, 0, sizeof(ip));
    char inf[IFNAMSIZ];
    memset(inf, 0, sizeof(inf));
    get_ip_and_inf(ip,inf);    
    fprintf(fp,"ip=%s\n",ip);// 备份主机ip地址   
    fprintf(fp,"inf=%s\n",inf);// 备份主机网络接口名称

    // 建立网桥
    char cmd1[100];
    sprintf(cmd1,"/usr/sbin/brctl addbr %s",br_name);   
    fprintf(fp,"bridge=%s\n",br_name);// 备份网桥名称
    printf("建立网桥:[%s]\n",br_name);
    system(cmd1);
    // TODO 执行结果判断
    
    // 活动接口加入网桥    
    char cmd2[100];
    sprintf(cmd2,"/usr/sbin/brctl addif %s %s",br_name,inf);
    printf("将活动接口[%s]加入网桥[%s]\n",inf,br_name);
    system(cmd2);
    
    // 清理活动接口地址
    char cmd3[100];
    sprintf(cmd3,"/sbin/ip addr flush %s",inf);
    printf("清理接口[%s]地址\n",inf);
    system(cmd3);
    
    // 配置网桥地址
    char cmd4[100];
    sprintf(cmd4,"/sbin/ifconfig %s %s",br_name,ip);
    printf("将IP[%s]注入网桥[%s]\n",ip,br_name);
    system(cmd4);
    
    // 配置默认路由
    char cmd5[100];
    char gw[16];
    memset(gw, 0, sizeof(gw));
    char *ptr,c='.';
    ptr = strrchr(ip,c);
    strncpy(gw,ip,ptr-ip+1);
    gw[ptr-ip+1]='\0';
    strcat(gw,"1");
    sprintf(cmd5,"/sbin/route add -net default gw %s",gw);
    fprintf(fp,"gw=%s\n",gw);// 备份默认网关
    printf("配置默认路由中...\n");
    system(cmd5);
    
    fclose(fp);
    /* ------ 初始化运行环境完毕 ------ */

    

	
	vmin = atoi(_vmin);
	vmax = atoi(_vmax);
//	if(0==strlen(rip) || 0==strlen(lip) || 0==strlen(tun_name)){
	if(0==strlen(rip) || 0==strlen(tun_name)){
		printf("params missed ! check configure file !\n");
		return -1;
	}

	/*
	if(argc != 4)
	{
		printf("usage: otv tap_name local_ip remote_ip\n");
		return -1;
	}*/

	s = socket(AF_INET, SOCK_DGRAM, 0);

	if(s < 0)
	{
		perror("socket");
		return -1;
	}

	//if(init_tun(argv[1]) < 0)
	if(init_tun(tun_name) < 0)
	{
		//printf("error: init_tun\n");
		return -1;
	}


  	printf("%s\n",tun_name);

        
  
	local.sin_family = AF_INET ;
	local.sin_port = htons(8888) ;
	//local.sin_addr.s_addr = INADDR_ANY ;
	//local.sin_addr.s_addr = inet_addr(argv[1]);
	local.sin_addr.s_addr = inet_addr(ip); // socket 绑定本地端口错误会导致数据包到达网卡接口后socket无法收到数据包

	addr.sin_family = AF_INET ;
	addr.sin_port = htons(8888) ;
	//addr.sin_addr.s_addr = inet_addr(argv[3]);
	addr.sin_addr.s_addr = inet_addr(rip);

	if(bind(s, (struct sockaddr *)&local, sizeof(local)) < 0)
	{
		perror("bind");
		return -1;
	}
	
	
	// TODO
	// 检测网桥 bridge 是否存在
    /* --- 启动隧道设备 --- */
    char cmd6[100];
    sprintf(cmd6,"/usr/sbin/brctl addif %s %s",br_name,tun_name);
    printf("将隧道接口[%s]加入网桥[%s]\n",tun_name,br_name);
    system(cmd6);
    
    
    char cmd7[100];
    sprintf(cmd7,"/sbin/ifconfig %s up",tun_name);
    printf("将隧道接口[%s]启动中...\n",tun_name);
    system(cmd7);
    /* --- 启动完毕 --- */
	
	pthread_create(&h, NULL, receiver, NULL);
	pthread_detach(h);

	char buf[2048];
	unsigned int net_protocol;
	unsigned int vlan_id=0;
	int n1=0x0f;
	int n2=0xff;
	struct ether_header *ether;
	while(1)
	{
		int ret;
		ret = read(tun0, buf, sizeof(buf));
		if (ret < 0)
		{
			perror("read tap0:");
			return -1;
		}	
		
		// analyse
    ether=(struct ether_header*)buf;
    //printf("------ Dest Mac ------\n");
    //printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",ether->ether_dhost[0],ether->ether_dhost[1],ether->ether_dhost[2],ether->ether_dhost[3],ether->ether_dhost[4],ether->ether_dhost[5]); // buf[0] ~ buf[5]
    //printf("------ Src  Mac ------\n");
    //printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",ether->ether_shost[0],ether->ether_shost[1],ether->ether_shost[2],ether->ether_shost[3],ether->ether_shost[4],ether->ether_shost[5]); // buf[6] ~ buf[11]
    
    net_protocol=ntohs(ether->ether_type);
    switch(net_protocol){
        case 0x0806:
            //printf("\n===> arp ! ");
            break;
        case 0x0800:
            //printf("\n===> ip ! ");
            break;
        case 0x8100:
            //printf("\n===> vlan !");
            vlan_id=buf[14]&n1;
            //printf("buf[14] %x ,buf[14]<<8 %x, buf[15] %x\n",buf[14]&p,vlan_id<<8,buf[15]&n);
            vlan_id = vlan_id<<8;
            vlan_id = vlan_id | (buf[15]&n2);
            //printf("vlan_id hex %x\n",vlan_id);
            printf("\n===> vlan_id dec %d",vlan_id);
            if(vlan_id >= vmin && vlan_id <= vmax){
                printf("read %10d bytes [ vlan %d ]", ret, vlan_id);
		            int n = sendto(s, buf, ret, 0, (struct sockaddr*)&addr, sizeof(addr));
		            if(n < 0)
		            {
			              perror("send socket:");
			              return -1;
		            }
            }
            break;
        default:
            printf("\n===> other! ");
    }
		
	}
	close(s);

	return 0;
}
