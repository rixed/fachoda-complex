#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
void main() {
	struct hostent *hp;
	int i,j;
	hp=gethostbyname("mephisto");
	printf("%s\n",hp->h_name);
	i=0;
	while (hp->h_aliases[i]!=NULL) printf("alias: %s\n",hp->h_aliases[i++]);
	printf("len=%d\n",hp->h_length);
	i=0;
	while (hp->h_addr_list[i]!=NULL) {
		for (j=0; j<hp->h_length; j++)
			printf("addr[%d]: %d\n",j,hp->h_addr_list[i][j]);
		i++;
	}
}

