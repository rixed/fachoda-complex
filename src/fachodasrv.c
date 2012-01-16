#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>
#define MYPORT 3490
#define MAXDATASIZE (1024*4)
#define MAXPACKETSIZE (1024*10)
struct sockaddr_in their_addr[NBMAXCLIENTS];
int new_fd[NBMAXCLIENTS];
FILE *Fnew_fd[NBMAXCLIENTS];
int nbh;
long int seed,DT,DTTotal=0;
struct timeval GTime,tv;
char Rec=0; FILE *Recf;
void closeall() {
	int h;
	for (h=0; h<nbh; h++) close(new_fd[h]);
}
static void catchalarm(int sig) {
	(void)sig;
	if (Rec==1) {
		Rec=2;
		printf("Interrupting recv\n");
		ungetc(12,Recf);	// pour débloquer recv
	} else {
		printf("Alarm, but not during a read !?\n");
	}
//	return catchalarm;
}
int recvall(int s, void *b, int l) {
	int n=0,e;
	do {
		e=recv(s,b,l-n,0);
		if (e!=-1) n+=e;
		else perror("recv");
	} while (Rec!=2 && e!=-1 && n!=l);
	if (n==l) return n; else return -1;
}
int main(int narg,char **arg) {
	int sockfd;
	struct sockaddr_in my_addr;
	int numbytes,h=0,hh, NbAllowedHosts;
	char buf[MAXDATASIZE];
	char packet[NBMAXCLIENTS][MAXPACKETSIZE];
	int packetlenght[NBMAXCLIENTS];
	char *deadclient;
	struct {
		float x,y,t; char b;
	} deadpacket = {0,0,0,0};
	int nbactiveclient;
	// parsing
	if (narg!=2 || sscanf(arg[1],"%d",&NbAllowedHosts)!=1) { printf("Syntax : fachodasrv NbAllowedHosts\n"); exit(1); }
	seed=0x5DEECE66;
	if (signal(SIGALRM,catchalarm)==SIG_ERR) { printf("sgnal error"); exit(-1); }
	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(MYPORT);
	my_addr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if (bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1) {
		perror("bind"); exit(1);
	}
	if (listen(sockfd,10)==-1) {
		perror("listen"); exit(1);
	}
	printf("It's open, clients are welcome !\n");
	do {
		printf("Still waitin' %d host%s... ",NbAllowedHosts-nbh,NbAllowedHosts-nbh>1?"s":""); fflush(stdout);
		socklen_t sin_size=sizeof(struct sockaddr_in);
		if ((new_fd[nbh]=accept(sockfd,(struct sockaddr*)&their_addr[nbh],&sin_size))==-1) {
			perror("accept"); continue;
		}
		if ((Fnew_fd[nbh]=fdopen(new_fd[nbh],"w"))==NULL) {perror("fdopen"); exit(-1); }
	//	if (fcntl(new_fd[nbh],F_SETFL,O_NONBLOCK)==-1) {perror("fcntl"); exit(-1); }
		printf("Got a connection from %s\n",inet_ntoa(their_addr[nbh].sin_addr));
		// envoyer le message d'acceuil
		if (send(new_fd[nbh],"m3dS\n",5,0)==-1) {
			perror("send"); closeall(); exit(1);
		}
		// recevoir l'ack
		printf("receiving ack\n");
		if ((numbytes=recvall(new_fd[nbh], buf, 5))<0) {
			closeall(); exit(1);
		}
		if (memcmp(buf,"m3dC\n",5)) {	// différents !?
			buf[numbytes]='\0';
			printf("%s do not acknoledge - Say him goodbye\n",buf);
			closeall(); exit(1);
		}
		// envoit le seed
		printf("sending seeds\n");
		if (send(new_fd[nbh],&seed,sizeof(seed),0)==-1) {
			perror("send"); closeall(); exit(1);
		}
		// envoit le nbr d'hosts
		if (send(new_fd[nbh],&NbAllowedHosts,sizeof(NbAllowedHosts),0)==-1) {
			perror("send"); closeall(); exit(1);
		}
		// envoit le numéro d'host
		if (send(new_fd[nbh],&nbh,sizeof(nbh),0)==-1) {
			perror("send"); closeall(); exit(1);
		}
		nbh++;
	} while (nbh<NbAllowedHosts);
	printf("Running with %d host%s...\n",nbh,nbh>1?"s":"");
	// informer des noms & camps & nbbot & viewall & navion de chacun
	for (h=0; h<nbh; h++) {
		if (recvall(new_fd[h], &packetlenght[h], sizeof(int))<0 ||
		    recvall(new_fd[h], &packet[h][0], packetlenght[h])<0 ||
		    recvall(new_fd[h], &packet[h][packetlenght[h]], sizeof(int))<0 ||
		    recvall(new_fd[h], &packet[h][packetlenght[h]+sizeof(int)], 5*sizeof(int))<0){exit(1);}
	}
	for (h=0; h<nbh; h++) {
		for (hh=0; hh<nbh; hh++) {
			if (h==hh) continue;
			if (send(new_fd[h],&packetlenght[hh],sizeof(int),0)==-1 ||
			    send(new_fd[h],&packet[hh][0],packetlenght[hh]+sizeof(int)*6,0)==-1) {perror("send"); exit(1); }
		}
	}
	gettimeofday(&GTime,NULL);
	deadclient=calloc(nbh,sizeof(char));
	nbactiveclient=nbh;
	do {
		// l'heure
		gettimeofday(&tv,NULL);
		if (GTime.tv_sec==tv.tv_sec) {
			DT=tv.tv_usec-GTime.tv_usec;
			GTime.tv_usec=tv.tv_usec;
		} else {
			DT=1000000*(tv.tv_sec-GTime.tv_sec)+tv.tv_usec-GTime.tv_usec;
			GTime.tv_sec=tv.tv_sec;
			GTime.tv_usec=tv.tv_usec;
		}
		DTTotal+=DT;
		//printf("DT=%ld\n",DT);
		// boucle sur tous les clients...
		for (h=0; h<nbh; h++) {
			// attend la fin du run (si ca vient, sinon suprime le joueur)
		//	printf("wainting end of turn from %d\n",h);
			if (!deadclient[h]) {
				Rec=1; Recf=Fnew_fd[h]; alarm(3);
				if (recvall(new_fd[h], buf, 4)<0){
					if (Rec==2) {
						printf("timeout expired : removing client #%d\n",h);
						deadclient[h]=1; nbactiveclient--;
					} else exit(-1);
				}
				Rec=0;
				if (memcmp(buf,"NAW\n",4)) {	// différents !?
					buf[4]='\0';
					printf("Hosts#%d : invalid flag (%s)\n",h,buf);
					deadclient[h]=1; nbactiveclient--;
				}
			}
			// recoit les datas
			packetlenght[h]=3*sizeof(float)+sizeof(char);
		//	printf("receiving %d bytes of datas from %d\n",packetlenght[h],h);
			if (!deadclient[h]) {
				Rec=1; Recf=Fnew_fd[h]; alarm(3);
				if (recvall(new_fd[h], &packet[h][0], packetlenght[h])<0) {
					if (Rec==2) {
						printf("timeout expired : removing client #%d\n",h);
						deadclient[h]=1; nbactiveclient--;
						memcpy(&packet[h][0],&deadpacket,packetlenght[h]);
					} else exit(-1);
				}
				Rec=0;
			} else {
				memcpy(&packet[h][0],&deadpacket,packetlenght[h]);
			}
			//printf("Received %d objects from host %d, size %d\n",nbo,h,packetlenght[h]);
		}
		// reboucle sur les clients...
		for (h=0; h<nbh; h++) {
			if (deadclient[h]) continue;
			// envoit le temps
		//	printf("sending time (%d) to clients #%d\n",DT,h);
			if (send(new_fd[h],&DT,sizeof(DT),0)==-1) {
				perror("send"); exit(1);
			}
			// et les packets des autres clients
			//	printf("sending data to #%d\n",h);
			for (hh=0; hh<nbh; hh++) {
				if (hh==h) continue;	// sauf les siens
				if (send(new_fd[h],&packet[hh],packetlenght[hh],0)==-1){perror("send");exit(1);}
			}
		}
	} while (nbactiveclient>0);
	printf("No more clients, exiting server\n");
	free(deadclient);
	close(sockfd);
	signal(SIGALRM,SIG_DFL);
	return(0);
}
