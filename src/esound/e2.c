#include <stdio.h>
#include <esd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define SIZETOTO 63433
int main() {
	int samp,esd;
	char *toto;
	FILE *fil;
	printf("audio devices=%s\n",esd_audio_devices());
	printf("audio device=%s  - audio rate=%d - audio format=%d\n",esd_audio_device,esd_audio_rate,esd_audio_format);
	esd_audio_format=ESD_BITS8|ESD_MONO|ESD_STREAM;
	esd_audio_rate=8000;
	printf("open return=%d\n",esd=esd_audio_open());
	toto=(char*)malloc(SIZETOTO);
	fil=fopen("toto.raw","r");
	fread(toto,SIZETOTO,1,fil);
	fclose(fil);
	esd_audio_write(toto,SIZETOTO);
	esd_audio_close();
	exit(0);
}
