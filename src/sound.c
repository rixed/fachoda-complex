#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "3d.h"

char sound=1;
char GUS=1;
FILE *dsp;
uchar *(sample[50]);	/// 30 samp max
int samplesize[50];
#define NBVOICES 10
int sampforvoice[NBVOICES];	// 10 voiex maxi
int esound_sampid[50];
char esound_samploop[50];
int nbsamp=0;

#if 0
SEQ_DEFINEBUF (2048);
#endif
int seqfd, gus_dev;

int esound_sock;
int esound_init() {
	return -1;
#	if 0
	int i;
	if ((esound_sock=esd_open_sound(NULL))<0) { perror("sock"); return -1; }
	for (i=0; i<NBVOICES; i++) sampforvoice[i]=-1;
	return 0;
#	endif
}

void seqbuf_dump () {
#	if 0
	if (_seqbufptr)
	if (write (seqfd, _seqbuf, _seqbufptr) == -1) { perror ("write /dev/sequencer"); exit (-1); }
	_seqbufptr = 0;
#	endif
}
int openseq() {
	return -1;
#	if 0
	int n, i;
	struct synth_info info;
	if ((seqfd = open("/dev/sequencer", O_WRONLY, 0))==-1){ perror("/dev/sequencer"); return -1; }
	if (ioctl(seqfd, SNDCTL_SEQ_NRSYNTHS, &n) == -1) { perror("/dev/sequencer"); return -1; }
	for (i = 0; i < n; i++) {
		info.device = i;
		if (ioctl (seqfd, SNDCTL_SYNTH_INFO, &info)==-1) { perror("/dev/sequencer"); return -1; }
		if (info.synth_type==SYNTH_TYPE_SAMPLE && info.synth_subtype==SAMPLE_TYPE_GUS) gus_dev=i;
	}
	if (gus_dev == -1) { fprintf(stderr,"Error: Gravis Ultrasound not detected\n"); return -1; }
	return 0;
#	endif
}
void gusreset() {
#	if 0
	if (!sound) return;
	ioctl (seqfd, SNDCTL_SEQ_SYNC, 0);
	ioctl (seqfd, SNDCTL_SEQ_RESETSAMPLES, &gus_dev);
#	endif
}
int opensound() {
	return -1;
#	if 0
	if (!sound) return 0;
	if (GUS && openseq()!=-1) {
		printf("GUS or alike detected\n");
		GUS_NUMVOICES (gus_dev,14);
		gusreset();
	} else {
		if (esound_init()<0) {
			sound=0;
			return -1;
		}
		printf("Sound OK using EsounD\n");
	}
	return 0;
#	endif
}
int loadsample(sample_e samp, char *fn, char loop) {
	return -1;
#	if 0
	struct patch_info *pat;
	long sr;
	FILE *in;
	nbsamp++;
	if (!sound) return 0;
	in=fopen(fn,"r");
	fseek(in,0,SEEK_END);
	sr=ftell(in);
	fseek(in,0,SEEK_SET);
	if (GUS) {
		pat=malloc(sr+sizeof(struct patch_info));
		pat->key=GUS_PATCH;
		pat->device_no=gus_dev;
		pat->instr_no=samp;
		pat->mode=WAVE_UNSIGNED;
		if (loop) pat->mode|=WAVE_LOOPING;
		pat->len=sr;
		pat->loop_start=0;
		pat->loop_end=(sr-1);
		pat->base_freq=8000;
		pat->base_note=440000;
		pat->high_note=200000000;
		pat->low_note=0;
		pat->panning=0;
		pat->detuning=0;
		pat->volume=120;
		fread(pat->data,1,sr,in);
		SEQ_WRPATCH(pat, sr+sizeof(struct patch_info));
		free(pat);
	} else {	// EsounD
		int length, total=0, sample_id=0, confirm_id, reget_sample_id;
		struct stat source_stats;
		char buf[ESD_BUF_SIZE];
		stat(fn,&source_stats);
		sample_id=esd_sample_cache(esound_sock,ESD_BITS8|ESD_MONO|ESD_STREAM|ESD_PLAY,8000,source_stats.st_size,fn);
		printf("sample id is <%d> whose name is %s\n",sample_id,fn);
		while ((length=fread(buf,1,ESD_BUF_SIZE,in))>0) {
			if ((length=write(esound_sock,buf,length))<=0) {
				fclose(in);
				return -1;
			}
			else total+=length;
		}
		
		confirm_id = esd_confirm_sample_cache(esound_sock);
		if (sample_id!=confirm_id) {
			printf("error while caching sample <%d>: confirm returned %d\n",sample_id,confirm_id);
		}
		
		reget_sample_id=esd_sample_getid(esound_sock,fn);
		printf("reget of sample %s id is <%d>\n",fn,reget_sample_id);
		if ((reget_sample_id != sample_id) || reget_sample_id < 0) {
			printf( "sample id's do not make sense!\n");
		}
		
		printf("sample uploaded, %d bytes\n",total);
		esound_sampid[samp]=reget_sample_id;
		esound_samploop[samp]=loop;
	}
	fclose(in);
	return 0;
#	endif
}

void sndplay(int v, int s, int n, int vol, int pan) {
	if (!sound || !vol) return;
	if (vol>127) vol=127;
#	if 0
	if (GUS) {
		SEQ_SET_PATCH(gus_dev, v, s);
		SEQ_PANNING(gus_dev, v, pan);
		SEQ_PITCHBEND (gus_dev, v, 0);
		SEQ_START_NOTE(gus_dev, v, n, vol);
		SEQ_DUMPBUF();
	} else {
		printf("playing sound %d, esound id = <%d>\n",s,esound_sampid[s]);
		if (sampforvoice[v]!=-1) if (esd_sample_stop(esound_sock,esound_sampid[sampforvoice[v]])<0) perror("sample stop in sndplay");
		sampforvoice[v]=s;
		if (esound_samploop[s]) {
			if (esd_sample_loop(esound_sock,esound_sampid[s])<0) perror("esound loop");
		} else {
			if (esd_sample_play(esound_sock,esound_sampid[s])<0) perror("esound play");
		}
	}
#	endif
}
void pitchbend (int v, int pit) {	// regler d'abord le BENDER_RANGE !
	if (!sound || !GUS) return;
#	if 0
	SEQ_PITCHBEND(gus_dev, v, pit);
	SEQ_DUMPBUF();
#	endif
}
void playsound(int v, sample_e samp, float freq, float vol, int pan) {	// pan entre -128 et 127
	if (!sound) return;
	sndplay(v,samp,70*freq,127*vol,pan);
#	if 0
	if (GUS) SEQ_DUMPBUF();
#	endif
}
void stopsound(int v, sample_e samp, float vol) {
	if (!sound) return;
#	if 0
	if (GUS) {
		SEQ_STOP_NOTE(gus_dev,v,samp,127*vol);
	} else {
		if (esd_sample_stop(esound_sock,esound_sampid[samp])<0) perror("sample kill");
	}
#	endif
}
void exitsound() {
	if (!sound) return;
//	SEQ_STOP_NOTE(gus_dev, 0, 0, 127);
#	if 0
	if (GUS) {
		SEQ_DUMPBUF();
		close(seqfd);
	} else {
		int i;
		for (i=0; i<nbsamp; i++) {
			esd_sample_stop(esound_sock,esound_sampid[i]);
			esd_sample_free(esound_sock,esound_sampid[i]);
		}
		close(esound_sock);
	}
#	endif
}
