#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "proto.h"
#define DEST_PORT 3490	// ces trois valeurs doivent correspondre a gate.c
#define MAXDATASIZE (1024*4)
#define MAXPACKETSIZE (1024*10)
char buf[MAXDATASIZE];
char packet[MAXPACKETSIZE];
int packetlenght;
short int packetnbo;
int numbytes;
int sockfd; FILE *sockfdF;
long int DT=0, TotalDT=0;
struct timeval GTime;

// signal
char Rec=0; FILE *Recf;
void catchalarm(int sig) {
	(void)sig;
	if (Rec==1) {
		Rec=2;
		printf("Interrupting recv\n");
		ungetc(12,Recf);	// pour débloquer recv
	} else {
		printf("Alarm, but not during a read !?\n");
	}
}

// fonctions timing de remplacement en monomode
void inittime(void) {
	gettimeofday(&GTime,NULL);
}
void deltatime(void) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	if (GTime.tv_sec==tv.tv_sec) {
		DT=tv.tv_usec-GTime.tv_usec;
		GTime.tv_usec=tv.tv_usec;
	} else {
		DT=1000000*(tv.tv_sec-GTime.tv_sec)+tv.tv_usec-GTime.tv_usec;
		GTime.tv_sec=tv.tv_sec;
		GTime.tv_usec=tv.tv_usec;
	}
	TotalDT+=DT;
}

/// fonctions reseau

int recvall(int s, void *b, int l) {
	int n=0,e;
	do {
		e=recv(s,b,l-n,0);
		if (e!=-1) n+=e;
		else perror("recv");
	} while (Rec!=2 && e!=-1 && n!=l);
	if (n==l) return n; else return -1;
}
int NetInit(char *hostname) {
	struct hostent *hp;
	long int seed;
	long int addy=0;
	int i;
	struct sockaddr_in their_addr;
	if ((hp=gethostbyname(hostname))==NULL) {printf("host error\n"); return(-1);}
	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	their_addr.sin_family=AF_INET;
	their_addr.sin_port=htons(DEST_PORT);
	for (i=0; i<hp->h_length; i++) {addy+=(hp->h_addr_list[0][i]&0xFF)<<(i<<3);}
	their_addr.sin_addr.s_addr=addy;//inet_addr("127.0.0.1");
	bzero(&(their_addr.sin_zero),8);
	if (connect(sockfd,(struct sockaddr*)&their_addr,sizeof(struct sockaddr))==-1) {
		perror("connect"); return(-1);
	}
	if ((sockfdF=fdopen(sockfd,"w"))==NULL) {perror("fdopen"); return(-1); }
	if ((numbytes=recvall(sockfd, buf, 5))==-1) {
		perror("recv"); return(-1);
	}
	if (memcmp(buf,"m3dS\n",5)) {	// différents !?
		buf[numbytes]='\0';
		printf("Serveur do not acknoledge (%s)\n",buf);
		close(sockfd); return(-1);
	}
	printf("Serveur OK\n");
	// renvoit la pareille
	if (send(sockfd,"m3dC\n",5,0)==-1) {
		perror("send"); return(-1);
	}
	// lit le seed
	if ((numbytes=recvall(sockfd, &seed, sizeof(seed)))==-1) {
		perror("recv"); return(-1);
	}
	srand48(seed);
	srand((int)seed);
	// lit le nbr d'Hosts
	if ((numbytes=recvall(sockfd, &NbHosts, sizeof(NbHosts)))==-1) {
		perror("recv"); return(-1);
	}
	printf("There will be %d hosts in the play\n",NbHosts);
	// lit le numéro d'host que je suis
	if ((numbytes=recvall(sockfd, &bmanu, sizeof(bmanu)))==-1) {
		perror("recv"); return(-1);
	}
	printf("I am host number %d\n",bmanu);
	return(0);
}
int NetCamp(void) {
	int o,i,j,k,l,s;
	if (MonoMode) return 0;
	// envoie notre nom
	o=strlen(myname)+1;
	if (send(sockfd,&o,sizeof(int),0)==-1) {perror("send"); return(-1);}
	if (send(sockfd,&myname,o,0)==-1) {perror("send"); return(-1);}
	// envoie notre camp
	if (send(sockfd,&camp,sizeof(int),0)==-1) {perror("send"); return(-1);}
	// envoie NBBOT & NBTANKSBOTS & ViewAll & SpaceInvaders & monvion
	if (send(sockfd,&NBBOT,sizeof(int),0)==-1 || send(sockfd,&NBTANKBOTS,sizeof(int),0)==-1 || send(sockfd,&ViewAll,sizeof(int),0)==-1 || send(sockfd,&SpaceInvaders,sizeof(int),0)==-1 || send(sockfd,&monvion,sizeof(int),0)==-1) {perror("send"); return(-1);}
	// qd tout le monde en a fait autant, on avance
	printf("waiting other players...\n");
	for (o=0; o<NbHosts; o++) {
		if (o==bmanu) continue;
		if (recvall(sockfd, &i, sizeof(int))==-1 ||
		    recvall(sockfd, &(playbotname[o]), i)==-1 ||
		    recvall(sockfd, &bot[o].camp, sizeof(int))==-1 ||
		    recvall(sockfd, &k, sizeof(int))==-1 ||
		    recvall(sockfd, &j, sizeof(int))==-1 ||
			 recvall(sockfd, &l, sizeof(int))==-1 ||
			 recvall(sockfd, &s, sizeof(int))==-1 ||
			 recvall(sockfd, &bot[o].navion, sizeof(int))==-1) {perror("recv"); return(-1);}
		bot[o].navion--;
		if (k<NBBOT) NBBOT=k;
		if (j<NBTANKBOTS) NBTANKBOTS=j;
		if (l) printf("## CAUTION : Player %s is using viewall mode ! ##\n",playbotname[o]);
		if (s && !SpaceInvaders) {
			SpaceInvaders=1;
			printf("## As wanted by %s, switching in SpaceInvaders mode ##\n",playbotname[o]);
		}
	}
	return 0;
}

int NetRead(void) {
	int o;
	if (MonoMode) {
		deltatime();
		return 0;
	}
	// prend l'incrément de temps et les commandes
	// 1) le temps
	// lire le temps
	Rec=1; Recf=sockfdF; alarm(5*NbHosts);
	if (recvall(sockfd, &DT, sizeof(DT))==-1) {
		if (Rec==2) {
			printf("timeout expired : server died ?\n");
			return -1;
		} else return -1;
	}
	Rec=0;
//	printf("just received time %ld\n",DT);
	// Les commandes des playbots
	for (o=0; o<NbHosts; o++) {
		if (o==bmanu) continue;
//		printf("receiving data for playbot %d\n",o);
		Rec=1; Recf=sockfdF; alarm(5*NbHosts);
		if (recvall(sockfd, &bot[o].xctl, sizeof(float))==-1 ||
		    recvall(sockfd, &bot[o].yctl, sizeof(float))==-1 ||
		    recvall(sockfd, &bot[o].thrust, sizeof(float))==-1 ||
			 recvall(sockfd, &bot[o].but, sizeof(char))==-1) {
			if (Rec==2) {
				printf("timeout expired : server died ?\n");
				return -1;
			} else return -1;
		}
		Rec=0;
	}
//	printf("EODebRun\n");
	return(0);
}
void NetSend(void) {
	if (MonoMode) return;
	packetlenght=0;
	memcpy(&packet[packetlenght],&bot[bmanu].xctl,sizeof(float)); packetlenght+=sizeof(float);
	memcpy(&packet[packetlenght],&bot[bmanu].yctl,sizeof(float)); packetlenght+=sizeof(float);
	memcpy(&packet[packetlenght],&bot[bmanu].thrust,sizeof(float)); packetlenght+=sizeof(float);
	memcpy(&packet[packetlenght],&bot[bmanu].but,sizeof(char)); packetlenght+=sizeof(char);
	// upload les transfo
	if (send(sockfd,"NAW\n",4,0)==-1) {perror("send"); exit(-1);}
	if (send(sockfd,&packet,packetlenght,0)==-1) { perror("send"); exit(-1); }
}
int NetClose(void) {
	if (MonoMode) return 0;
	printf("Link to server closed\n");
	close(sockfd);
	return(0);
}

