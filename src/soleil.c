#include <math.h>
#include "3d.h"
#define SolMapX 64	// pour le soleil
#define SolMapY 32
#define SolImgL 40
#define NBETOILES 80
pixel *SolMap;
double SolPicPh[20], SolPicOmega[20];
int *SolImgAdy;
int phix;

struct {
	float x,y,z,xy;
	char type;
} *etoile;

void initsol() {
	int x,y,i;
	double r,teta;
	SolMap = malloc(sizeof(pixel)*SolMapX*(SolMapY+1));
	SolImgAdy = malloc(sizeof(int)*SolImgL*SolImgL);
	for (y=-SolImgL/2; y<SolImgL/2; y++)
		for (x=-SolImgL/2; x<SolImgL/2; x++) {
			r = sqrt(x*x+y*y);
			teta = x ? (atan(-y/(double)x) + (x<0 ? M_PI : 0)) : (y>0 ? -M_PI/2 : M_PI/2);
			while (teta<0 || teta>2*M_PI) {	// A REFAIRE AVEC FMOD !
				if (teta<0) teta+=2*M_PI;
				if (teta>2*M_PI) teta -= 2*M_PI;
			}
			if (r<SolImgL/2)
				SolImgAdy[(y+SolImgL/2)*SolImgL+x+SolImgL/2] = SolMapX*(int)(SolMapY*r/(SolImgL*.5)) + SolMapX*teta/(2*M_PI);
			else
				SolImgAdy[(y+SolImgL/2)*SolImgL+x+SolImgL/2] = -1;
		}
	for (i=0; i<20; i++) { SolPicPh[i] = randK()*2*M_PI; SolPicOmega[i]=randK()*0.4+0.3; }
	phix=0;
	// les étoiles
	if ((etoile=malloc(NBETOILES*sizeof(*etoile)))==NULL) {perror("malloc init3d");exit(-1);}
	for (i=0; i<NBETOILES; i++){
		double teta,phi;
		do teta=randK()*M_PI*.4; while (randK()>sin(teta));
		phi=randK()*M_PI*2;
		etoile[i].x=1e6*sin(teta)*cos(phi);
		etoile[i].y=1e6*sin(teta)*sin(phi);
		etoile[i].z=1e6*cos(teta);
		etoile[i].xy=etoile[i].x*etoile[i].y;
		etoile[i].type=i&3;
	}
}

void animsoleil() {
	int i, SolNbPic=9;
	for (i=0; i<SolNbPic; i++) SolPicPh[i]+=randK();
}
void affsoleil(vector *L) {
	vector soleil;
	int i,x,y,xse,yse,a, SolNbPic=9;
	vector ac;
	float ab,de,gh;
	gh = obj[0].rot.z.x*obj[0].rot.z.y;
	ab = obj[0].rot.x.x*obj[0].rot.x.y;
	de = obj[0].rot.y.x*obj[0].rot.y.y;
	if (Dark) for (i=0; i<NBETOILES; i++) {
		// calcul la position de l'etoile dans la caméra, ie CamT*Etoile
		ac.z = /*2.**/((obj[0].rot.z.x+etoile[i].y)*(obj[0].rot.z.y+etoile[i].x)-gh-etoile[i].xy+obj[0].rot.z.z*etoile[i].z);
		if (ac.z > 0) {
			ac.x = (obj[0].rot.x.x+etoile[i].y)*(obj[0].rot.x.y+etoile[i].x)-ab-etoile[i].xy+obj[0].rot.x.z*etoile[i].z;
			x = (ac.x*focale)/ac.z;
			if (x>=-_DX && x<_DX-1) {
				ac.y = (obj[0].rot.y.x+etoile[i].y)*(obj[0].rot.y.y+etoile[i].x)-de-etoile[i].xy+obj[0].rot.y.z*etoile[i].z;
				y = (ac.y*focale)/ac.z;
				if (y>=-_DY && y<_DY-1) {
					//plot(x-_DX,y-_DY,0xFFFFFF);
					plot(x,y-1,0xA0A0A0);
					plot(x+1,y-1,0x909090);
					plot(x-1,y,0xA0A0A0);
					plot(x,y,0xF0F0F0);
					plot(x+1,y,0xB0B0B0);
					plot(x+2,y,0x808080);
					plot(x-1,y+1,0x909090);
					plot(x,y+1,0xB0B0B0);
					plot(x+1,y+1,0xA0A0A0);
					plot(x+2,y+1,0x909090);
					plot(x,y+2,0x808080);
					plot(x+1,y+2,0x606060);
				}
			}
		}
	}
	copyv(&soleil,L);
	neg(&soleil);
	mulv(&soleil,1e10);
	// quelle position pour le soleil ?
	ac.z = scalaire(&obj[0].rot.z,&soleil);
	if (ac.z > 0) {
		ac.x = scalaire(&obj[0].rot.x,&soleil);
		xse = _DX+(ac.x*focale)/ac.z-SolImgL/2;
		if (xse>=-SolImgL && xse<SX) {
			ac.y = scalaire(&obj[0].rot.y,&soleil);
			yse = _DY+(ac.y*focale)/ac.z-SolImgL/2;
			if (yse>=-SolImgL && yse<SY) {
				// AFICHAGE DU SOLEIL
				if (!Dark) for (i=0; i<SolNbPic; i++) {
					for (x=(i*SolMapX)/SolNbPic; x<((i+1)*SolMapX)/SolNbPic; x++) {
						for (y=0; (y<SolMapY*.3+SolMapY*.3*(sin(SolPicPh[i])+1.5)*sin(x*M_PI*SolNbPic/SolMapX)*sin(x*M_PI*SolNbPic/SolMapX)) && y<SolMapY; y++) {
							SolMap[y*SolMapX+x].r = (255-3*y);
							SolMap[y*SolMapX+x].g = (255-5*y);
							SolMap[y*SolMapX+x].b = ((200-10*y)>0?(200-10*y):0);
						}
						for (; y<SolMapY; y++) {
							*(int*)&SolMap[y*SolMapX+x] = 0xA0A0C0;
						}
					}
				} else for (i=0; i<SolNbPic; i++) {
					for (x=(i*SolMapX)/SolNbPic; x<((i+1)*SolMapX)/SolNbPic; x++) {
						for (y=0; (y<SolMapY*.5+SolMapY*.1*(sin(SolPicPh[i])+1.5)*sin(x*M_PI*SolNbPic/SolMapX)*sin(x*M_PI*SolNbPic/SolMapX)) && y<SolMapY; y++) {
							SolMap[y*SolMapX+x].r = y<SolMapY*.5?255-3*y:170-3*y;
							SolMap[y*SolMapX+x].g = y<SolMapY*.5?255-3*y:170-3*y;
							SolMap[y*SolMapX+x].b = y<SolMapY*.5?255-y:170-3*y;
						}
						for (; y<SolMapY; y++) {
							*(int*)&SolMap[y*SolMapX+x] = 0x202040;
						}
					}
				}
				for (y=0; y<SolImgL; y++) {
					if (y+yse>=0 && y+yse<SY) for (x=0; x<SolImgL; x++) if (x+xse>=0 && x+xse<SX) {
						a=SolImgAdy[y*SolImgL+x];
						if (a!=-1) {
							videobuffer[(y+yse)*SX+x+xse].r=SolMap[a].r;
							videobuffer[(y+yse)*SX+x+xse].b=SolMap[a].b;
							videobuffer[(y+yse)*SX+x+xse].g=SolMap[a].g;
						}
					}
				}
			}
		}
	}
}
