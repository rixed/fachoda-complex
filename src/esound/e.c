#include <stdio.h>
#include <esd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

int main() {
	int sock, length, total=0;
	int sample_id = 0;
	FILE *source = NULL;
	struct stat source_stats;
	char *name="../snd/pingouin.raw";
	char buf[ESD_BUF_SIZE];
	
	if ((source=fopen( name, "r" ))<0) perror("fopen");
	if ((sock=esd_open_sound(NULL))<0) perror("sock");
	if (sock<=0) return 1;
	stat(name,&source_stats);
	sample_id = esd_sample_cache(sock,ESD_BITS8|ESD_MONO|ESD_STREAM|ESD_PLAY,8000,source_stats.st_size,name);
	printf("sample id is <%d>\n",sample_id);
	
	while ((length=fread(buf,1,ESD_BUF_SIZE,source))>0) {
		if ((length=write(sock,buf,length))<= 0) return 1;
		else total+=length;
	}
	printf( "sample uploaded, %d bytes\n",total);
	
	sample_id = esd_sample_cache(sock,ESD_BITS8|ESD_MONO|ESD_STREAM|ESD_PLAY,8000,source_stats.st_size,"blabla");
	printf("sample id is again <%d> !!!\n",sample_id);
	exit(-1);
	
	if (esd_sample_loop(sock,sample_id)<0) perror("loop");
	
	printf( "press <enter> to quit.\n" );
	getchar();
	
	/* TODO: make sample_free clean out all playing samples */
	esd_sample_stop( sock, sample_id );
	esd_sample_free( sock, sample_id );
	
	printf( "closing down\n" );
	close( sock );
	
	return 0;
}
