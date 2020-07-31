#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <time.h>
#include <signal.h>

#ifndef MAXLEN
#define MAXLEN 1024
#endif


char sendbuf[MAXLEN];
unsigned long int ip;
unsigned int port;
time_t starttime;

unsigned long int my_aton(char *str)
{       int n1,n2,n3,n4;
        unsigned long ip;
        n1=n2=n3=n4=0;
        sscanf(str,"%d.%d.%d.%d",&n1,&n2,&n3,&n4);
        n1=n1&0xff; n2=n2&0xff;
        n3=n3&0xff; n4=n4&0xff;
        ip=(n1<<24)+(n2<<16)+(n3<<8)+n4;
        return ip;
}

char *getsn(void) {
	static char sn[MAXLEN];
	char buf[MAXLEN];
	FILE *fp=NULL;
	sn[0]=0;	
	fp=popen("/sbin/ip link show eth0","r");
	if(fp==NULL) return sn;
	while(fgets(buf,MAXLEN,fp)) {
		if(strstr(buf,"link/ether")) {
			char *p,*mac;
			p=(char*)strstr(buf,"link/ether");
			if(p==(char*)0) continue;
			p+=11; mac=p;
			p=(char*)strstr(mac," brd ");
			if(p==(char*)0) continue;
			*p=0; p=sn;
			while(*mac) {
				if(*mac!=':') { *p=*mac; p++;
				}
				mac++;
			}	
			*p=0;
			pclose(fp);
			return sn;
		}
	}
	pclose(fp);
	return sn;
}



void getstat(void) {
	int len=0,ret;

	FILE *fp;
	char buf[MAXLEN];
	char *p;
/* hostname */
	fp= fopen("/etc/ethudp/HOSTNAME","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"hostname=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);

/* starttime */
	ret=snprintf(sendbuf+len,MAXLEN-len,"starttime=%ld\n",starttime);
        if(ret>0) len+=ret;

/* uptime */
	fp= fopen("/proc/uptime","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		long int upt;
		if(sscanf(buf,"%ld",&upt)==1) {
			ret=snprintf(sendbuf+len,MAXLEN-len,"uptime=%ld\n",upt);
			if(ret>0) len+=ret;
		};
	} 
	if(fp!=NULL) fclose(fp);

/* vpnindex */
	fp= fopen("/etc/ethudp/SITE/INDEX","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"index=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);

/* SN */
	fp= fopen("/etc/ethudp/SITE/SN","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"sn=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);

/* ZC */
	fp= fopen("/etc/ethudp/SITE/ZC","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"zc=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);

/* VPNSETUP VERSION */
	fp= fopen("/usr/src/vpnsetup/VERSION","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"vpnsetupversion=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);

/* EthUDP VERSION */
	fp= fopen("/usr/src/ethudp/VERSION","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"ethudpversion=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);

/* Temp*/
	fp= fopen("/sys/bus/platform/devices/coretemp.0/temp2_input","r");
	if((fp!=NULL) && (fgets(buf,MAXLEN,fp)!=NULL) ){
		ret=snprintf(sendbuf+len,MAXLEN-len,"temp=%s",buf);
		if(ret>0) len+=ret;
	}
	if(fp!=NULL) fclose(fp);
}


void sendstat()
{	int fd;
	struct sockaddr_in to;
	int len;

	getstat();

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) return ;
	len=sizeof(to);
        memset(&to, 0, len);
//      bind(fd, (struct sockaddr *)&to, len);
        to.sin_family = PF_INET;
        to.sin_port = htons(port);
        memcpy(&to.sin_addr, &ip, 4);
	sendto(fd,sendbuf,strlen(sendbuf),0,(struct sockaddr *)&to, sizeof(to));
	exit(0);
}

int main(int argc,char*argv[])
{
	setsid();
	signal(SIGCHLD,SIG_IGN);
	ip = my_aton("202.38.95.44");
	starttime=time(NULL);
	port = 6021;

	if(argc>=2) 
		ip = my_aton(argv[1]);
	else if(argc>=3) 
		port = atoi(argv[2]);
	ip=htonl(ip);
	if(fork()) exit(0);
	while(1) {
		if(fork()==0) {
			sendstat();
			exit(0);
		}
		sleep(60);	
	}
}
