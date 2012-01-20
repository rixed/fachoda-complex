#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "keycodesdef.h"
#include "proto.h"

extern void deltatime(void);
extern void plotfumee(int,int,int);
static struct {
	char nbkases;
	struct {
		char *label;
		char nxtround;
	} kase[9];
} Round []= {
	{	// cercle 0
		3,
		{
			{"Options",16},
			{"Quit",10},
			{"Go !",-1}
		}
	}, {	// cercle 1 game type
		8,
		{
			{"Strategy",4},
			{"What Plane",5},
			{"Number of Drones",-4},
			{"Number of Tanks",-5},
			{"Nation",6},
			{"Day or Night ?",7},
			{"When Dead...",15},
			{"Back",16}
		}
	}, {	// cercle 2 techniks
		5,
		{
			{"Controls",8},
			{"KbControl Sensitivity",-7},
			{"KbControl Amortisment",-8},
			{"Language",9},
			{"Back",16}
		}
	}, {	// cercle 3 Cheat modes
		4,
		{
			{"Difficulty",11},
			{"Super-Sight Cheat",12},
			{"More Cheats",13},
			{"Back",16}
		}
	} , {	// cercle 4 Initial Position
		2,
		{
			{"Campain",-14},
			{"Kill'em All",-15}
		}
	}, {	// cercle 5 Plane
		6,
		{
			{"Snophill",-37},
			{"Dewoitine",-16},
			{"Bogoplane",-17},
			{"Corsair",-18},
			{"Spitflame",-34},
			{"Moshito",-19}
		}
	}, {	// cercle 6 Camp
		4,
		{
			{"Sierra Freedom",-20},
			{"D.T.V.U.",-21},
			{"Almouchie",-22},
			{"Low-Wanana",-23}
		}
	}, {	// cercle 7 Time
		2,
		{
			{"Day",-24},
			{"Night",-25}
		}
	}, {	// cercle 8 Controles
		2,
		{
			{"Mouse",-26},
			{"Keyboard",-27}
		}
	}, {	// cercle 9 Language
		2,
		{
			{"English",-28},
			{"Francais",-29}
		}
	}, {	// cercle 10 Quit
		2,
		{
			{"Yeah, sure!",-3},
			{"Well, maybe not...",0}
		}
	}, {	// cercle 11 Difficulty
		2,
		{
			{"Easy",-11},
			{"Normal",-12}
		}
	}, {	// cercle 12 Visibility
		2,
		{
			{"Spot all enemies",-13},
			{"Spot only friends",-30}
		}
	}, {	// cercle 13 MetaGruge
		2,
		{
			{"Alti Cheat",-31},
			{"No, too lame",-32}
		}
	}, {	// cercle 14 Pas dans le menu, just un boutton QUIT
		1,
		{
			{"OK",16}
		}
	},	{	// cercle 15, Mortal/Immoral?
		2,
		{
			{"Replace a drone",-33},
			{"Rest In Peace",-6}
		}
	}, {	// cercle 16 Options
		6,
		{
			{"Game Type", 1},
			{"Technics", 2},
			{"Redefine Keys", -2},
			{"Cheat", 3},
			{"Map", 17},
			{"Back", 0}
		}
	},	{	// cercle 17 Map
		3,
		{
			{"Hilly", -35},
			{"Smoothness", -36},
			{"Back", 16}
		}
	}
};

void drawseg(int x1, int x2, int y, int c) {
	if (x1<0) x1=0;
	if (x2>SX-1) x2=SX-1;
	if (y>=0 && y<SY && x1<SX && x2>=0)
		MMXMemSetInt((int*)videobuffer+x1+y*SX,c,x2-x1+1);
}
void disqueintro(int x, int y, int r, int c) {
	int balance=-r, xoff=0, yoff=r, newyoff=1;
	if (r<=0 || x-r>=SX || x+r<0 || y-r>=SY || y+r<0 || r>SX) return;
//	MMXSaveFPU();
	do {
		if (newyoff) {
			drawseg(x-xoff,x+xoff,y+yoff,c);
			drawseg(x-xoff,x+xoff,y-yoff,c);
		}
		if (xoff!=yoff) {
			drawseg(x-yoff,x+yoff,y+xoff,c);
			if (xoff) drawseg(x-yoff,x+yoff,y-xoff,c);
		}
		if ((balance += xoff + xoff + 1) >= 0) {
			yoff --;
			balance -= yoff + yoff;
			newyoff=1;
		} else newyoff=0;
	} while (++xoff <= yoff);
//	MMXRestoreFPU();
}
#define RAYONBOUTTON 40
void button(int x, int y, char *label,char highlight) {
	disqueintro(x,y,RAYONBOUTTON,highlight?0x359FE0:0x258FD0);
	cercle(x-_DX,y-_DY,RAYONBOUTTON-3,0xD0D0D0);
	cercle(x-_DX,y-_DY,RAYONBOUTTON,0x103050);
	TextClipX1=x-RAYONBOUTTON+6;
	TextClipX2=x+RAYONBOUTTON-6;
	pstrlent(label,y-7,0xFFF0F0);
	TextClipX1=TextClipX2=0;
}

int xb[10],yb[10], kzc;
int agit=0;
void page(int r, float rayon, float phase) {
	int b;
	int SS=MAX(_DX,_DY);
//	MMXMemSetInt((int*)videobuffer,BACKCOLOR,SX*SY);
	affpresent(drand48()*(agit>>8),drand48()*(agit>>8));
	if (agit>256) agit=(agit*9)/10;
	for (b=0; b<Round[r].nbkases; b++) {
		float ang=phase+b*M_PI*2./Round[r].nbkases;
		xb[b]=_DX+SS*rayon*cos(ang);
		yb[b]=_DY+SS*rayon*sin(ang);
	}
	for (b=0; b<Round[r].nbkases; b++) {
		int dx=xmouse-xb[b];
		int dy=ymouse-yb[b];
	//	disqueintro(xb[b]-dx*.2,yb[b]-dy*.2,RAYONBOUTTON*(.6+sqrt(dx*dx+dy*dy)*.5/SS),0x486878);
		plotfumee(xb[b]-dx*.2,yb[b]-dy*.2,RAYONBOUTTON*(.6+sqrt(dx*dx+dy*dy)*.5/SS));
	}
	for (b=0; b<Round[r].nbkases; b++)
		button(xb[b],yb[b],Round[r].kase[b].label,kzc==b);
}
int kazeclick(int x, int y, int r) {
	int b;
	for (b=Round[r].nbkases-1; b>=0; b--) {
		if ((x-xb[b])*(x-xb[b])+(y-yb[b])*(y-yb[b])<RAYONBOUTTON*RAYONBOUTTON) return b;
	}
	return -1;
}
int jauge(int vi, int max) {
	int va=vi;
	int jx, y;
	float phaz=0;
	do {
		jx=(va*(SX-20))/max;
		kzc=kazeclick(xmouse,ymouse,14);
		page(14,.45+.2*sin(phaz*.61),.5*M_PI+0.2*sin(phaz));
		deltatime();
		AccelFactor=(float)DT/NORMALDT;
		phaz+=AccelFactor*.21;
		for (y=SY/3-(SY>>3); y<SY/3+(SY>>3); y++)
			MMXMemSetInt((int*)videobuffer+y*SX+10,0x3060A0,jx);
		pbignum(va,_DX,SY/3-SizeBigCharY/2,2,1,0);
		plotcursor(xmouse,ymouse);
		buffer2video();
		xproceed();
		if (kread(0) || kread(1)) {
			if (kzc==0) {
				playsound(VOICEMOTOR,BIPINTRO, 1+(drand48()-.5)*.05,1,(xmouse*128)/SX);
				agit=50*256;
				return va;
			}
			else if (xmouse>=10 && xmouse<SX-10) va=((xmouse-10)*max)/(SX-20);
		}
	} while (1);
}
/*void readstring(char *m) {
	float phaz=0;
	int curpos=strlen(m), i;
	char prompt[]="Type in name :                                             ";
	do {
		kzc=kazeclick(xmouse,ymouse,14);
		page(14,.45+.2*sin(phaz*.61),.5*M_PI+0.2*sin(phaz));
		phaz+=.11;
		for (i=0; i+15<strlen(prompt) && i<strlen(m)+1; i++)
		prompt[i+15]=i==curpos?108+16:m[i];
		prompt[i+15]='\0';
		pstr(prompt,SY/3,0xFFF020);
		plotcursor(xmouse,ymouse);
		buffer2video();
		//wait_sync();	!! INDISPENSABLE
		xproceed();
		if (kread(0) || kread(1)) {
			if (kzc==0) return;
		}

	} while (1);
}*/
int present() {
	int curround=0, oldcurround, nextround, etap=2;
	float phaz=drand48()*2*M_PI, rayon=2, phazr=drand48()*2*M_PI;
	inittime();
	do {
		deltatime();
		AccelFactor=(float)DT/NORMALDT;
		if (etap==1) {
			// explosion
			rayon+=AccelFactor*.23;
			if (rayon>1.5) {
				if (nextround<0) {
					switch (nextround) {
					case -1: return -1;
					case -2: return 1;
					case -4: NBBOT=jauge(NBBOT,100); break;
					case -5: NBTANKBOTS=jauge(NBTANKBOTS,500); break;
					case -7: CtlSensitiv=.01*jauge(100*CtlSensitiv,100); break;
					case -8: CtlAmortis=.01*jauge(100*CtlAmortis,100); break;
					case -6: redefinekeys(); break;
			/*		case -9: readstring(hostname);
					case -10: readstring(myname);*/
					case -11: Accident=300+exp(.1*jauge(log(Accident-300.)/.1,100)); break;
					case -12: Smooth=2+jauge(Smooth-2,20); break;
					}
				} else curround=nextround;
				etap=2;
			}
		} else if (etap==2) {
			// implosion
			rayon-=AccelFactor*.23;
			phaz+=AccelFactor*(.09+sin(phazr)*.61);
			phazr+=AccelFactor*(sin(phaz*.1)*.63+sin(phazr*.3)*.11);
			if (rayon<=.4) {
				etap=0;
				rayon=.4;
			}
		}
		kzc=kazeclick(xmouse,ymouse,curround);
		page(curround,rayon+.18*sin(phazr),phaz);
		plotcursor(xmouse,ymouse);
		buffer2video();
		xproceed();
		phaz+=AccelFactor*(.009+sin(phazr)*.061);
		phazr+=AccelFactor*(sin(phaz*.01)*.063+sin(phazr*.013)*.011);
		if (etap==0 && (kreset(0) || kreset(1))) {
			int b=kzc;
			if (b!=-1) {
				playsound(VOICEMOTOR,BIPINTRO, 1+(drand48()-.5)*.05,1,(xmouse*128)/SX);
				agit=20*256;
				if (Round[curround].kase[b].nxtround>=0) {
					oldcurround=curround;
					nextround=Round[curround].kase[b].nxtround;
					etap=1;
				}
				else {
					switch (Round[curround].kase[b].nxtround) {
					case -1: etap=1; nextround=-2; break;
					case -2: etap=1; nextround=-6; break;
					case -3: etap=1; nextround=-1; break;
					case -4: etap=1; nextround=-4; break;
					case -5: etap=1; nextround=-5; break;
					case -6: AllowResurrect=0; break;
					case -7: etap=1; nextround=-7; break;
					case -8: etap=1; nextround=-8; break;
				/*	case -9: etap=1; nextround=-9; break;
					case -10: etap=1; nextround=-10; break;*/
					case -11: Easy=1; break;
					case -12: Easy=0; break;
					case -13: ViewAll=1; break;
					case -14: SpaceInvaders=0; break;
					case -15: SpaceInvaders=1; break;
					case -16: monvion=1; break;
					case -17: monvion=4; break;
					case -18: monvion=2; break;
					case -19: monvion=3; break;
					case -20: camp=0; break;
					case -21: camp=1; break;
					case -22: camp=2; break;
					case -23: camp=3; break;
					case -24: Dark=0; break;
					case -25: Dark=1; break;
					case -26: MouseCtl=1; break;
					case -27: MouseCtl=0; break;
					case -28: lang=1; break;
					case -29: lang=0; break;
					case -30: ViewAll=0; break;
					case -31: Gruge=1; break;
					case -32: Gruge=0; break;
					case -33: AllowResurrect=1; break;
					case -34: monvion=5; break;
					case -35: etap=1; nextround=-11; break;
					case -36: etap=1; nextround=-12; break;
					case -37: monvion=6; break;
					}
					if (etap==0) {
						nextround=oldcurround;
						etap=1;
					}
				}
			}
		}
	} while (1);
}
