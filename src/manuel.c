#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <math.h>
#include "proto.h"
#include "map.h"
#include "keycodesdef.h"

int DogBot=0;
vector DogBotDir;
float DogBotDist;

void NextDogBot() {
	int DBi=DogBot;
	DogBotDist=0;
	do {
		if (++DogBot>=NBBOT) DogBot=0;
		if (DogBot!=bmanu) {
			copyv(&DogBotDir,&obj[bot[DogBot].vion].pos);
			subv(&DogBotDir,&obj[bot[bmanu].vion].pos);
			DogBotDist=renorme(&DogBotDir);
		}
	} while (DogBot!=DBi && (DogBotDist>DOGDISTMAX || bot[DogBot].camp==-1 || DogBot==bmanu));
}
void PrevDogBot() {
	int DBi=DogBot;
	DogBotDist=0;
	do {
		if (--DogBot<0) DogBot=NBBOT-1;
		if (DogBot!=bmanu) {
			copyv(&DogBotDir,&obj[bot[DogBot].vion].pos);
			subv(&DogBotDir,&obj[bot[bmanu].vion].pos);
			DogBotDist=renorme(&DogBotDir);
		}
	} while (DogBot!=DBi && (DogBotDist>DOGDISTMAX || bot[DogBot].camp==-1 || DogBot==bmanu));
}

void manuel(int b) {
	int i;
	vector u;
	xproceed();
	// BOUTON GAUCHE
	if (!mapmode) switch (arme) {
	case 0:
		if ((MouseCtl && kread(0)) || kread(gkeys[kc_fire].kc)) bot[b].but.canon=1;
		break;
	case 1:
		if ((MouseCtl && kreset(0)) || kreset(gkeys[kc_fire].kc)) bot[b].but.bomb=1;
		break;
	} else if ((MouseCtl && kread(0)) || kread(gkeys[kc_fire].kc)) {
		bot[b].u.x=((xmouse-_DX)*(WMAP/2)*ECHELLE)/zoom+xcarte*ECHELLE;
		bot[b].u.y=((_DY-ymouse)*(WMAP/2)*ECHELLE)/zoom+ycarte*ECHELLE;
//		printf("Z=%f\n",z_ground(bot[b].u.x,bot[b].u.y, true));
	}
	// BOUTON DROIT
	if ((MouseCtl && kreset(1)) || kreset(gkeys[kc_weapon].kc)) {
		if (abs(xmouse)<1 && abs(ymouse)<1) quitte++;
		arme^=1;
	}
	// Echappement
	if (quitte==1) {
		if (kreset(gkeys[kc_yes].kc)) quitte=2;
		if (kreset(gkeys[kc_no].kc)) quitte=0;
	} else if (kreset(gkeys[kc_esc].kc) && (bot[bmanu].camp!=-1 || !AllowResurrect || !resurrect())) quitte++;
	// Moteur
/*	if (kreset(gkeys[kc_motor0].kc)) bot[b].thrust=0;
	if (kreset(gkeys[kc_motor1].kc)) bot[b].thrust=.1;
	if (kreset(gkeys[kc_motor2].kc)) bot[b].thrust=.2;
	if (kreset(gkeys[kc_motor3].kc)) bot[b].thrust=.3;
	if (kreset(gkeys[kc_motor4].kc)) bot[b].thrust=.4;
	if (kreset(gkeys[kc_motor5].kc)) bot[b].thrust=.5;
	if (kreset(gkeys[kc_motor6].kc)) bot[b].thrust=.6;
	if (kreset(gkeys[kc_motor7].kc)) bot[b].thrust=.7;
	if (kreset(gkeys[kc_motor8].kc)) bot[b].thrust=.8;
	if (kreset(gkeys[kc_motor9].kc)) bot[b].thrust=.9;
	if (kreset(gkeys[kc_motor10].kc)) bot[b].thrust=1;*/
	if (kread(gkeys[kc_motormore].kc)) bot[b].thrust+=.05;
	if (kread(gkeys[kc_motorless].kc)) bot[b].thrust-=.05;
	// Vues
	if (kreset(gkeys[kc_externview].kc)) {	mapmode=0; if (++visu>5) visu=1; }
	if (kreset(gkeys[kc_internview].kc)) { mapmode=0; if (visu==0) visu=7; else visu=0; }
	if (kreset(gkeys[kc_travelview].kc)) {
		float zs;
		mapmode=0;
		visu=6;
		copyv(&obj[0].pos,&obj[bot[visubot].vion].rot.x);
		mulv(&obj[0].pos,300+drand48()*600+loinvisu);
		copyv(&u,&obj[bot[visubot].vion].rot.y);
		mulv(&u,(drand48()-.5)*600);
		addv(&obj[0].pos,&u);
		copyv(&u,&obj[bot[visubot].vion].rot.z);
		mulv(&u,(drand48()-.5)*600);
		addv(&obj[0].pos,&u);
		addv(&obj[0].pos,&obj[bot[visubot].vion].pos);
		if (obj[0].pos.z<(zs=z_ground(obj[0].pos.x,obj[0].pos.y, false)+100)) obj[0].pos.z=zs;
	}
	if (kreset(gkeys[kc_nextbot].kc)) {
		if (visu!=7) {
			do {
				if (++visubot>=NBBOT) visubot=0;
			} while (!ViewAll && bot[visubot].camp!=camp);	// pas bmanu.camp car peut etre tue
//			printf("visubot=%d\n",visubot);
			soundthrust=-1;
			if (bot[visubot].camp==-1) playsound(VOICEMOTOR,FEU,1,1,0);
		} else NextDogBot();
	}
	if (kreset(gkeys[kc_prevbot].kc)) {
		if (visu!=7) {
			do {
				if (--visubot<0) visubot=NBBOT-1;
			} while (!ViewAll && bot[visubot].camp!=camp);
//			printf("visubot=%d\n",visubot);
			soundthrust=-1;
			if (bot[visubot].camp==-1) playsound(VOICEMOTOR,FEU,1,1,0);
		} else PrevDogBot();
	}
	if (kreset(gkeys[kc_mybot].kc)) {
		if (visu!=7) visubot=b;
		else {
			float d;
			int DBi, DBm;
			NextDogBot();
			d=DogBotDist;
			DBi=DogBot; DBm=DogBot;
			do {
				NextDogBot();
				if (DogBotDist<d && bot[DogBot].camp!=bot[bmanu].camp) {
					d=DogBotDist;
					DBm=DogBot;
				}
			} while (DogBot!=DBi);
			DogBot=DBm; DogBotDist=d;
		}
	}
	if (kreset(gkeys[kc_nextbomb].kc)) { for (i=bot[visubot].vion; i<bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces && (obj[i].objref!=-1 || (obj[i].type!=BOMB)); i++); if (i<bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces) visubomb=i; else visubomb=0; }
	if (!accel || imgcount>64) {
			if (kread(gkeys[kc_zoomout].kc)) {
			if (!mapmode) loinvisu+=10;
			else zoom+=_DX/6;
		}
		if (kread(gkeys[kc_zoomin].kc)) {
			if (!mapmode) loinvisu-=10;
			else if ((zoom-=_DX/6)<_DX) zoom=_DX;
		}
		if (kread(gkeys[kc_riseview].kc)) {
			if (!mapmode) { if ((visuteta-=.2)<-M_PI) visuteta+=2*M_PI; }
			else if ((ycarte+=1+(3*SX)/zoom)>WMAP/2) ycarte=WMAP/2;
		}
		if (kread(gkeys[kc_lowerview].kc)) {
			if (!mapmode) { if ((visuteta+=.2)>M_PI) visuteta-=2*M_PI; }
			else if ((ycarte-=1+(3*SX)/zoom)<-WMAP/2) ycarte=-WMAP/2;
		}
		if (kread(gkeys[kc_rightenview].kc)) {
			if (!mapmode) { if ((visuphi-=.2)<-M_PI) visuphi+=2*M_PI; }
			else if ((xcarte+=1+(3*SX)/zoom)>WMAP/2) xcarte=WMAP/2;
		}
		if (kread(gkeys[kc_leftenview].kc)) {
			if (!mapmode) { if ((visuphi+=.2)>M_PI) visuphi-=2*M_PI; }
			else if ((xcarte-=1+(3*SX)/zoom)<-WMAP/2) xcarte=-WMAP/2;
		}
	}
	if (visu!=7) {
		if (kreset(gkeys[kc_towardview].kc)) { visuteta=visuphi=0; }
		if (kreset(gkeys[kc_backview].kc)) { visuteta=0; visuphi=M_PI; }
		if (kreset(gkeys[kc_leftview].kc)) { visuteta=0; visuphi=M_PI*.5; }
		if (kreset(gkeys[kc_rightview].kc)) { visuteta=0; visuphi=-M_PI*.5; }
		if (kreset(gkeys[kc_upview].kc)) { visuteta=-M_PI/2; visuphi=0; }
	} else {
		tournevisu=0;
		if (kread(gkeys[kc_towardview].kc)) { tournevisu=1; visuteta=visuphi=0; }
		if (kread(gkeys[kc_backview].kc)) { tournevisu=1; visuteta=0; visuphi=M_PI; }
		if (kread(gkeys[kc_leftview].kc)) { tournevisu=1; visuteta=0; visuphi=M_PI*.5; }
		if (kread(gkeys[kc_rightview].kc)) { tournevisu=1; visuteta=0; visuphi=-M_PI*.5; }
		if (kread(gkeys[kc_upview].kc)) { tournevisu=1; visuteta=-M_PI/2; visuphi=0; }
		if (!tournevisu) visuteta=visuphi=0;
	}
	avancevisu=kread(gkeys[kc_movetowardview].kc);
	// Boutons de commandes
	if (kreset(gkeys[kc_gear].kc)) bot[b].but.gear^=1;
	if (kreset(gkeys[kc_flaps].kc)) {
		bot[b].but.flap^=1;
		playsound(VOICEGEAR,BIPBIP,1,1,0);
	}
	bot[b].but.frein=kread(gkeys[kc_brakes].kc);
	if (kreset(gkeys[kc_buziness].kc)) bot[b].but.commerce=1;
	if (kreset(gkeys[kc_autopilot].kc)) {
		autopilot^=1;
		playsound(VOICEGEAR,BIPBIP,1,1,0);
	}
	// Control du jeu
	if (MonoMode && kreset(gkeys[kc_pause].kc)) {
		inittime();
		lapause^=1;
	}
	AfficheHS=kread(gkeys[kc_highscores].kc);
	if (kreset(gkeys[kc_accelmode].kc) && MonoMode) { accel^=1; imgcount&=63; }
	if (kreset(gkeys[kc_basenav].kc)) {
		bot[b].u.x=obj[bot[b].babase].pos.x;
		bot[b].u.y=obj[bot[b].babase].pos.y;
	}
	if (kreset(gkeys[kc_mapmode].kc)) {
		mapmode^=1;
		playsound(VOICEGEAR,BIPBIP3,1,1,0);
	}
	if (kreset(gkeys[kc_suicide].kc) && bot[bmanu].camp!=-1) explose(bot[visubot].vion,0);
	if (kreset(gkeys[kc_markpos].kc)) bot[b].but.repere=1;
	// Cheats
	if (Gruge && kread(gkeys[kc_alti].kc)) obj[bot[visubot].vion].pos.z+=100;
	if (Gruge && kreset(gkeys[kc_gunned].kc)) /*bot[visubot].gunned=bmanu;*/ hitgun(zep[0].o,zep[0].o);
	if (!autopilot && !mapmode) {
		if (MouseCtl) {
			bot[b].xctl=((xmouse-_DX)/(double)_DX);
			bot[b].yctl=((ymouse-_DY)/(double)_DY);
		} else {
			int i=0;
			i=kread(gkeys[kc_left].kc);
			i+=kread(gkeys[kc_right].kc)<<1;
			i+=kread(gkeys[kc_down].kc)<<2;
			i+=kread(gkeys[kc_up].kc)<<3;
			if (i) {
				CtlSensActu+=CtlSensitiv*AccelFactor;
				if (i&1) bot[b].xctl-=CtlSensActu;
				if (i&2) bot[b].xctl+=CtlSensActu;
				if (i&4) bot[b].yctl-=CtlSensActu;
				if (i&8) bot[b].yctl+=CtlSensActu;
			} else CtlSensActu=0;
			if (bot[b].xctl<-1 || bot[b].xctl>1 || bot[b].yctl<-1 || bot[b].yctl>1) CtlSensActu=0;
			if (!(i&3)) bot[b].xctl*=CtlAmortis;
			if (!(i&12)) bot[b].yctl=CtlYequ+(bot[b].yctl-CtlYequ)*CtlAmortis;
			if (kread(gkeys[kc_center].kc)) {
				bot[b].xctl=0;
				bot[b].yctl=CtlYequ;
				if (kread(gkeys[kc_down].kc) && CtlYequ>-1) CtlYequ-=.02;
				if (kread(gkeys[kc_up].kc) && CtlYequ<1) CtlYequ+=.02;
			}
		}
	} else {
		float zs=z_ground(obj[bot[b].vion].pos.x,obj[bot[b].vion].pos.y, true);
		if (obj[bot[b].vion].pos.z-zs>500) {
			double dc=cap(bot[b].u.x-obj[bot[b].vion].pos.x,bot[b].u.y-obj[bot[b].vion].pos.y)-bot[b].cap;
			float a,n,vit=scalaire(&bot[b].vionvit,&obj[bot[b].vion].rot.x);
			if (dc<-M_PI) dc+=2*M_PI;
			else if (dc>M_PI) dc-=2*M_PI;
			if (autopilot) {
				if (dc>.5) a=-.9;
				else if (dc<-.5) a=.9;
				else a=-.9*dc/.5;
			} else a=0;
			bot[b].xctl=(a-obj[bot[b].vion].rot.y.z);
			if (autopilot) {
				if (vit<40) bot[b].thrust+=.01;
				else if (vit>40 && bot[b].thrust>.02) bot[b].thrust-=.01;
			}
			n=9000+zs-obj[bot[b].vion].pos.z;
			if (zs<6000) {
				if (zs>1000) n+=12000-2*zs; else n+=20000-5*zs;
			}
			n=5*atan(1e-3*n);
			bot[b].yctl=(n-bot[b].vionvit.z)*.13;
		} else {
			bot[b].xctl=-obj[bot[b].vion].rot.y.z;
			bot[b].yctl=0;
		}
	}
/*	if (bot[b].xctl<-1) bot[b].xctl=-1;
	if (bot[b].xctl>1) bot[b].xctl=1;
	if (bot[b].yctl<-1) bot[b].yctl=-1;
	if (bot[b].yctl>1) bot[b].yctl=1;
	{
		matrix m;
		double ct,st,cf,sf;
		ct=cos(-bot[b].xctl*.5);
		st=sin(-bot[b].xctl*.5);
		cf=cos(bot[b].yctl*.4+.2);
		sf=sin(bot[b].yctl*.4+.2);
		m.x.x=cf;		m.y.x=sf*st;		m.z.x=-sf*ct;
		m.x.y=0;			m.y.y=ct;			m.z.y=st;
		m.x.z=sf;		m.y.z=-st*cf;		m.z.z=cf*ct;
		copym(&obj[bot[b].vion+43].rot,&m);
	}*/
}
