#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <values.h>
#include "map.h"

void MMXMemSetInt(int *deb, int coul, int n) {
	while (n--) *deb++ = coul;
}

void MMXCopy(int *dst, int *src, int n) {	// by the time I suppose glibc became quite good at this :-)
	memcpy(dst, src, n*4);
}
void MMXFlat(int *dst, int n, int coul) {
	MMXMemSetInt(dst, coul, n);
}
void MMXSaveFPU(void) {}
void MMXRestoreFPU(void) {}
void MMXFlatInit(void) {}
void MMXPhongInit(int aa, int intcol) { (void)aa; (void)intcol; }

extern void catchalarm(int sig);

matrix mat_id={{1,0,0},{0,1,0},{0,0,1}};
vector vac_diag={1,1,1}, vec_zero={0,0,0}, vec_g={0,0,-1};
// 3DSTUDIO
int NBBOT=30;
int NBTANKBOTS=150;
modele *mod;
objet *obj;
int nbobj, debtir, firstfumee, fumeedispo, DebMoulins, FinMoulins;
float AngleMoulin=0;
uchar *rayonfumee;
uchar *typefumee;
char hostname[250]="localhost";
int fumeesource[NBFUMEESOURCE], fumeesourceintens[NBFUMEESOURCE];
double focale;
int camp=1, AllowResurrect=1, Easy=0, Gruge=0, ViewAll=0, SpaceInvaders=0, monvion=1, lang=1, Dark=-1, Fleuve=1, MouseCtl=1, Accident=1500, Smooth=7;
float CtlSensitiv=0.08, CtlSensActu=0, CtlAmortis=.9, CtlYequ=.1;
alienpos_s *alienpos;
char myname[30];
matrix LightSol={ {-.7,0,-.7},{0,1,0},{.7,0,-.7}};
matrix Light;
vector ExplozePos; int Exploze=0;
//
int babaseo[2][3][4];	// o1, o2 // base 1, base 2, base 3 // camp A,B,C,D
int gunner[NBMAXTIR]; short int vieshot[NBMAXTIR];
bombe_s *bombe; int bombidx;
char (*playbotname)[30];
debris_s debris[NBGRAVMAX];
// options changeables à la ligne de com :
char WINDOW=1;
char PHONG=1;
float TROPLOIN=120.,TROPLOIN2;
float AccelFactor=1;
int _DX,_DY,SX=400,SY=250,SYTB,SXTB,SIZECERCLE,POLYMAX=25,TBY;
int nbtir;
// ajoute un objet dans le world
void addobjet(int mo, vector *p, matrix *m, int or, uchar sol) {
	int xk,yk,ak;
	obj[nbobj].model=mo;
	obj[nbobj].type=mod[mo].type;
	obj[nbobj].aff=1;
	copyv(&obj[nbobj].pos,p);
	if (sol) obj[nbobj].pos.z+=z_ground(p->x,p->y, true);
	copym(&obj[nbobj].rot,m);
	obj[nbobj].objref=or;
	xk=(int)floor(p->x/ECHELLE)+(WMAP>>1);
	yk=(int)floor(p->y/ECHELLE)+(WMAP>>1);
	ak=xk+(yk<<NWMAP);
	obj[nbobj].ak=ak;
	obj[nbobj].next=map[ak].first_obj;
	map[ak].first_obj=nbobj;
	obj[nbobj].prec=-1;
	if (obj[nbobj].next!=-1) obj[obj[nbobj].next].prec=nbobj;
	nbobj++;
	//calcposaind(nbobj-1);
}
void tournevion(int v, float d, float p, float g) {
	float ar, ap, ag, c1, c2, c3, s1, s2, s3;
	matrix r;
	ar=d*2.; ap=p*.5; ag=g*.5;
	c1=cos(ar); s1=sin(ar); c2=cos(ap); s2=sin(ap); c3=cos(ag); s3=sin(ag);
/*	r.x.x=c2;		r.y.x=0;		r.z.x=-s2;
	r.x.y=-s1*s2;	r.y.y=c1;	r.z.y=-s1*c2;
	r.x.z=c1*s2;	r.y.z=s1;	r.z.z=c1*c2;
	*/
	r.x.x=c2*c3;				r.y.x=-c2*s3;				r.z.x=-s2;
	r.x.y=c1*s3-s1*s2*c3;	r.y.y=s1*s2*s3+c1*c3;	r.z.y=-s1*c2;
	r.x.z=c1*s2*c3+s1*s3;	r.y.z=s1*c3-s3*c1*s2;	r.z.z=c1*c2;
	mulm(&obj[v].rot,&r);
}
void backgroundline(int *v,int sx,int dz,int z,int coul) {
	int x;
	uchar cz;
	if (coul==-1) return;
	if (coul==0) {
		if (z>64<<8) z=64<<8;
		else if (z<0) z=0;
		if (!Dark) for (x=0; x<sx; x++) {
			uchar r;
			cz=z>>8;
			r=224-cz;
			cz=-cz;	//?
			v[x]=(r<<16)+(r<<8)+(cz&0xFF);
			z+=dz;
		} else for (x=0; x<sx; x++) {
			uchar r;
			cz=z>>8;	// z>>8 entre 0 et 64
			r=0x20+64-cz;
			cz=0x40+64-cz;
			v[x]=(r<<16)+(r<<8)+(cz&0xFF);
			z+=dz;
		}
	} else {
		MMXMemSetInt(v,coul,sx);
	}
}
void background() {
	int z1,z2, dz1,dz2;
	int dz, x, i, xfin;
	int zfront[3] = {64<<8,0,-64<<8};
#define sol 0x1A6834
#define soldark (0x1A6834>>1)
#define ciel 0xA0A0C0
#define cieldark 0x202040
	int coulfront[2][4] = {
		{ciel,0,sol,-1},
		{cieldark,0,soldark,-1}
	};
	int *vid;
	MMXSaveFPU();
	z1=z2=(focale*cam.rot.z.z-_DY*cam.rot.y.z+30)*256;	// 30 pour descendre un peut la ligne d'horizon
	z1-=(_DX*cam.rot.x.z)*256;
	z2+=(_DX*cam.rot.x.z)*256;
	dz1=cam.rot.y.z*256;
	dz2=cam.rot.y.z*256;
	dz=(z2-z1)/SX;
#define ZFINSOL ((-SX/4)<<8)
	for (vid=(int*)videobuffer; vid<(int*)videobuffer+SX*SY; vid+=SX, z1+=dz1, z2+=dz2) {
		if (z1>z2) {
			for (i=0, x=0; i<3 && x<SX; i++) {
				if (z1>zfront[i]) {
					if (z2>zfront[i]) {
						xfin=SX;
						backgroundline(vid+x,xfin-x,dz,z1,coulfront[Dark][i]);
						x=xfin;
					} else {
						if (z1-z2!=0) {
							xfin=((z1-zfront[i])*SX)/(z1-z2);
							if (xfin>SX) xfin=SX;
						} else xfin=SX;
						backgroundline(vid+x,xfin-x,dz,z1,coulfront[Dark][i]);
						x=xfin;
					}
				}
			}
			if (x<SX) backgroundline(vid+x,SX-x,dz,z1,coulfront[Dark][i]);
		} else {
			for (i=2, x=0; i>=0 && x<SX; i--) {
				if (z1<zfront[i]) {
					if (z2<zfront[i]) {
						xfin=SX;
						backgroundline(vid+x,xfin-x,dz,z1,coulfront[Dark][i+1]);
						x=xfin;
					} else {
						if (z1-z2!=0) {
							xfin=((z1-zfront[i])*SX)/(z1-z2);
							if (xfin>SX) xfin=SX;
						} else xfin=SX;
						backgroundline(vid+x,xfin-x,dz,z1,coulfront[Dark][i+1]);
						x=xfin;
					}
				}
			}
			if (x<SX) backgroundline(vid+x,SX-x,dz,z1,coulfront[Dark][i+1]);
		}
	}
	MMXRestoreFPU();
}
void balanceX(int o, double s) {	// balance l'obj o poue lui mettre la tete en haut
	double d;
	vector yp;
	static vector ypt={0, -.099833416, .995004165};
	matrix m;
	mulmv(&obj[o].rot,&ypt,&yp);
	if (obj[o].rot.z.z>0) d = yp.z-obj[o].rot.z.z;
	else d = obj[o].rot.z.z-yp.z;
	copym(&m,&mat_id);
	m.y.y=cos(s*d);
	m.y.z=sin(s*d);
	m.z.y=-sin(s*d);
	m.z.z=cos(s*d);
	mulm(&obj[o].rot,&m);
}
void balanceY(int o, double s) {	// balance l'obj o poue lui mettre la tete en haut
	double d;
	vector yp;
	static vector ypt={.099833416, 0, .995004165};
	matrix m;
	mulmv(&obj[o].rot,&ypt,&yp);
	if (obj[o].rot.z.z>0) d = yp.z-obj[o].rot.z.z;
	else d = obj[o].rot.z.z-yp.z;
	copym(&m,&mat_id);
	m.x.x=cos(s*d);
	m.x.z=-sin(s*d);
	m.z.x=sin(s*d);
	m.z.z=cos(s*d);
	mulm(&obj[o].rot,&m);
}
void basculeY(int o, float a) {
	matrix m;
	copym(&m,&mat_id);
	m.x.x=cos(a);
	m.x.z=-sin(a);
	m.z.x=sin(a);
	m.z.z=cos(a);
	mulm(&obj[o].rot,&m);
}
void basculeX(int o, float a) {
	matrix m;
	copym(&m,&mat_id);
	m.y.y=cos(a);
	m.y.z=sin(a);
	m.z.y=-sin(a);
	m.z.z=cos(a);
	mulm(&obj[o].rot,&m);
}
void basculeZ(int o, float a) {
	matrix m;
	copym(&m,&mat_id);
	m.x.x=cos(a);
	m.x.y=sin(a);
	m.y.x=-sin(a);
	m.y.y=cos(a);
	mulm(&obj[o].rot,&m);
}
int visubot=0, visuobj=0;

int akpos(vector *p) {
	int x,y;
	x=p->x+((WMAP<<NECHELLE)>>1);
	y=p->y+((WMAP<<NECHELLE)>>1);
	x>>=NECHELLE;
	y>>=NECHELLE;
	return x+(y<<NWMAP);
}
void akref(int ak,vector *r) {
	int x=(ak&(WMAP-1))-(WMAP>>1);
	int y=(ak>>NWMAP)-(WMAP>>1);
	r->x=(x<<NECHELLE);
	r->y=(y<<NECHELLE);
	r->z=z_ground(r->x,r->y, true);
}
int resurrect(void) {	// jesus revient, jesus reviuent parmis les tiens...
	int j=NBBOT, jj, bestprix=0;
	bot_s bottmp;
	for (jj=NbHosts; jj<NBBOT; jj++) {
		if (bot[jj].camp==camp && viondesc[bot[jj].navion].prix<=viondesc[bot[bmanu].navion].prix && viondesc[bot[jj].navion].prix>bestprix) {
			bestprix=viondesc[bot[jj].navion].prix;
			j=jj;
		}
	}
	if (j<NBBOT) {
		memcpy(&bottmp,&bot[bmanu],sizeof(bot_s));
		memcpy(&bot[bmanu],&bot[j],sizeof(bot_s));
		memcpy(&bot[j],&bottmp,sizeof(bot_s));
		bot[bmanu].gold=55;
		playsound(VOICEEXTER,ALLELUIA,1,1,0);
		soundthrust=-1;
		autopilot=1;
		accel=0;
		xcarte=obj[bot[bmanu].vion].pos.x/ECHELLE;
		ycarte=obj[bot[bmanu].vion].pos.y/ECHELLE;
		return 1;
	}
	return 0;
}

int visu=0, visubomb=0, mapmode=0, accel=0, autopilot=0, lapause=0, lepeintre=0, bmanu, imgcount=0;
double loinvisu=110, visuteta=0,visuphi=0;
uchar avancevisu=0, tournevisu=0, quitte=0, arme=0, AfficheHS=0;
int main(int narg, char **arg) {
	int i,j, dtradio=0, RedefineKeys=0; vector p; matrix m;
	int caisse=0, dtcaisse=0, oldgold=0, caissetot=0, maxgold=0, initradio=0;
	char *userid;
	vector posc[2], OldCamDep;
	float angvisu1=0,n;
	int maxrank=20;
	FILE *file;
	HS_s highscore[20]={
		{ 5000, "Jack l'Eventreur"},
		{ 4000, "Titi Poutine"},
		{ 3500, "Oncle Picsou"},
		{ 3000, "John Royce junior"},
		{ 2500, "Rock fait l'air"},
		{ 2000, "Celui qui a dit non"},
		{ 1800, "L'ami Beria"},
		{ 1600, "Mo bout tout"},
		{ 1400, "Mite Errante"},
		{ 1200, "Heider de der"},
		{ 1000, "Jaruzelsky La Viande Froide"},
		{  900, "Bokassa la Barique"},
		{  800, "Le Gros Timonier"},
		{  700, "Tonton Makoute"},
		{  600, "Nunuche Phouet Boigny"},
		{  500, "Foccard Fondu de plomb"},
		{  400, "Hassan le Prophete"},
		{  300, "Paf le chien"},
		{  200, "Abbe de la Sainte Gamelle"},
		{  100, "marine craccRa"}
	};
//	vect2dc p1={0,-60,{150,200,40}},p2={40,20,{40,220,50}},p3={-50,80,{100,10,240}},p4={0,0,{240,40,140}}; float bolop=0;
	printf(" Fachoda Complex v2 - (c) Cedric Cellier, april 2000\n"
"\n"
"   fullscreen      : play in DGA mode instead of windowed mode\n"
"   x n             : X size of the window (default : 320)\n"
"   y n             : Y size (default : 200)\n"
"   night           : Play at night\n"
"   camp 1|2|3|4    : the camp you want to fly for (default : 1)\n"
"   drone n         : number of drones (default : 30)\n"
"   tank n          : total number of tanks (default : 200)\n"
"   host name       : name of the machine that runs the server (default : localhost)\n"
"   mortal          : resurections are forbiden (default : come back as soon as dead just like the little Jesus)\n"
"   name            : your name in the game (default : your user id)\n"
"   easy            : easy mode (default : guess)\n"
"   viewall         : view all enemies on the map (default : no)\n"
"   nosound         : turn sound OFF (default : sound on)\n"
"   killemAll       : Kill em All !!! (default : just be kool, this is a game)\n"
"   plane n         : The plane you start with : 1 for Dewoitine, 2 for Corsair, etc (default : 2)\n"
"   french          : Pour que les textes soient en francais (defaut : frenglish)\n"
"   keys            : Redefine the keys and save in file '.keys'\n"
"   nogus           : force the use of /dev/dsp instead of /dev/sequencer\n"
"   xcolor          : for X11 version, let X converts pixel values (slower, but can solve palette errors on some video cards)\n"
"   gruge           : Who knows ?\n"
);
	/*
	    Who Am I ?
	  */
	userid = getlogin();
	snprintf(myname, sizeof(myname), "%s", userid ? userid : "an unknown");

	/*
	    Command line parser
	                         */
	for (i=1; i<narg; i++) {
		int c=0;
		while (arg[i][c]=='-' || arg[i][c]==' ') c++;
		if (!strcmp(&arg[i][c],"fullscreen")) WINDOW=0;
		else if (!strcmp(&arg[i][c],"night")) Dark=1;
		else if (!strcmp(&arg[i][c],"x")) {
			if (++i==narg || sscanf(arg[i],"%d",&SX)!=1) goto parse_error;
		} else if (!strcmp(&arg[i][c],"y")) {
			if (++i==narg || sscanf(arg[i],"%d",&SY)!=1) goto parse_error;
		} else if (!strcmp(&arg[i][c],"camp")) {
			if (++i==narg || sscanf(arg[i],"%d",&camp)!=1 || camp<1 || camp>4) goto parse_error;
		} else if (!strcmp(&arg[i][c],"drone")) {
			if (++i==narg || sscanf(arg[i],"%d",&NBBOT)!=1 || NBBOT<0 || NBBOT>100) goto parse_error;
		} else if (!strcmp(&arg[i][c],"tank")) {
			if (++i==narg || sscanf(arg[i],"%d",&NBTANKBOTS)!=1 || NBTANKBOTS<1 || NBTANKBOTS>500) goto parse_error;
		} else if (!strcmp(&arg[i][c],"host")) {
			if (++i==narg || sscanf(arg[i],"%s",hostname)!=1) goto parse_error;
		} else if (!strcmp(&arg[i][c],"mortal")) AllowResurrect=0;
		else if (!strcmp(&arg[i][c],"name")) {
			if (++i==narg) goto parse_error; else {
				for (j=0; j<(int)strlen(arg[i]) && j<29; j++) myname[j]=arg[i][j];
				myname[j]='\0';
				}
		} else if (!strcmp(&arg[i][c],"easy")) Easy=1;
		else if (!strcmp(&arg[i][c],"viewall")) ViewAll=1;
		else if (!strcmp(&arg[i][c],"nosound")) sound=0;
		else if (!strcmp(&arg[i][c],"nomouse")) MouseCtl=0;
		else if (!strcmp(&arg[i][c],"killemall")) SpaceInvaders=1;
		else if (!strcmp(&arg[i][c],"plane")) {
			if (++i==narg || sscanf(arg[i],"%d",&monvion)!=1 || monvion<1 || monvion>NBNAVIONS) goto parse_error;
		} else if (!strcmp(&arg[i][c],"french")) lang=0;
		else if (!strcmp(&arg[i][c],"keys")) RedefineKeys=1;
		else if (!strcmp(&arg[i][c],"gruge")) Gruge=1;
		else if (!strcmp(&arg[i][c],"nogus")) GUS=0;
		else if (!strcmp(&arg[i][c],"xcolor")) XCONVERT=1;
		else {
parse_error:
			printf("Something was wrong in your command line...\n");
			exit(1);
		}
	}
	camp--;
	initradio=4;
	if (SX<200) SX=250;
	if (SY<200) SY=200;
//	TROPLOIN2=TROPLOIN*TROPLOIN;
	SX&=0xFFFFFFF8; SY&=0xFFFFFFFE;
	_DX=SX>>1; _DY=SY>>1;
//	SIZECERCLE=MIN(SX,SY)/10;
	SYTB=90;//SY/4;
	TBY=SY-SYTB;
	SXTB=SYTB*2; //SX>>1;
	focale=_DX;
	if (!WINDOW && XCONVERT) {
		WINDOW=1;
		printf("Can't let X11 converts the pixel values in DGA mode\n");
	}
	/* lire les highscore */
	if ((file=fopen(".highscores","r"))!=NULL) {
		fread(&highscore,sizeof(HS_s),20,file);
		fclose(file);
	}
	/* autres inits */
	loadfont("font.tga", 16,7, 10);
	loadbigfont("bigfont.tga");
	initrender();
	loadtbtile("wood50_50.tga");
	tbback1=tbback;
	loadtbtile("metal50_50.tga");
	tbback2=tbback;
	initmapping();
	initsol();
	for (i=0; i<NBREPMAX; i++) repere[i].x=MAXFLOAT;
	if (opensound()==-1) printf("Ce sera le monde du silence...\n");
	/*
	    VIDEO
	           */
	videobuffer=(pixel32*)malloc(SX*SY*sizeof(pixel32));
	BufVidOffset=SX*sizeof(pixel32);
	initvideo();
	//	printf("Img bpp=%d\n",depth);
	drawtbback();
/*	fontname=XListFonts(disp,"-freefont-cooper-*-*-*-*-*-*-100-*-*-*-*-*",1,&i);
	xfont=XLoadQueryFont(disp,fontname[0]);
	XFreeFontNames(fontname);
	XSetState(disp,gc,0xFFFF00,0,GXcopy,0xFFFFFF);
	XSetFont(disp,gc,xfont->fid);*/
	/* KEYS */
	if ((file=fopen(".keys","r"))!=NULL) {
		printf("Load custom keys\n");
		for (i=0; i<NBKEYS; i++) fread(&gkeys[i].kc,sizeof(char),1,file);
		fclose(file);
	}
	// PRESENTATION
	loadsample(PRESENT,"snd/pingouin.raw",0);
	animpresent();
	if (RedefineKeys) {
		redefinekeys();
		goto fin;
	}
	loadsample(BIPINTRO,"snd/bipintro.raw",0);
	if (present()==-1) goto fin;
	//MMXMemSetInt((int*)videobuffer,BACKCOLOR,SX*SY);
	affpresent(0,0);
	pstr("LOADING and CREATING THE WORLD",_DY+(SY>>3)+10,0xE5D510);
	/* SOUND */
	loadsample(SHOT,"snd/shot.raw",0);
	loadsample(GEAR_DN,"snd/gear_dn.raw",0);
	loadsample(GEAR_UP,"snd/gear_up.raw",0);
	loadsample(SCREETCH,"snd/screetch.raw",0);
	loadsample(MOTOR,"snd/motor.raw",1);
	loadsample(HIT,"snd/hit.raw",0);
	loadsample(MESSAGE,"snd/message.raw",0);
	loadsample(EXPLOZ,"snd/exploz.raw",0);
	loadsample(EXPLOZ2,"snd/explo1.raw",0);
	loadsample(TOLE,"snd/tole.raw",0);
	loadsample(BIPBIP,"snd/bipbip.raw",0);
	loadsample(BIPBIP2,"snd/bipbip2.raw",0);
	loadsample(BIPBIP3,"snd/bipcarte.raw",0);
	loadsample(FEU,"snd/feu.raw",1);
	loadsample(TARATATA,"snd/taratata.raw",0);
	loadsample(ALLELUIA,"snd/alleluia.raw",0);
	loadsample(ALERT,"snd/alert.raw",0);
	loadsample(PAIN,"snd/pain.raw",0);
	loadsample(DEATH,"snd/death.raw",0);
	loadsample(BRAVO,"snd/bravo.raw",0);
	/*
	    contact le serveur
		                   */
	if (signal(SIGALRM,catchalarm)==SIG_ERR) { printf("signal error"); exit(-1); }
	if (NetInit(hostname)==-1) {
		printf("Monomode\n");
		NbHosts=1;
		MonoMode=1;
	} else MonoMode=0;
	if (!MonoMode) {
		Easy=0;
		AllowResurrect=0;
		printf("Multiplayer game ; disabling easy mode & resurrection\n");
	}
	NBBOT+=NbHosts;
	playbotname=malloc(30*NbHosts);
	strcpy(&(playbotname[bmanu])[0],myname);
	if (Dark==-1) Dark=drand48()>.9;
	Fleuve=drand48()>.01;
	/*
	    Load les modèles
		                   */
	LoadModeles();
	/*
	    init le world
		                */
	initworld();
	// etre RAPIDE après, jusqu'au prochain NetSend, car le serveur
	// attend la marque de fin de tour avec un timeout de 3 secondes !
//	printf("World is now generated (%d objs) ; let it now degenerate !\n",nbobj);
/*	for (dtradio=j=k=l=i=0; i<nbobj; i++) {
		j+=mod[obj[i].model].nbpts[0];
		k+=mod[obj[i].model].nbfaces[0];
		l+=obj[i].objref==-1;
		dtradio+=mod[obj[i].model].fixe==-1;
	}
	printf("Taille d'un objet : %d octets (%.3f Mo pour tous)\n",sizeof(objet),sizeof(objet)*(double)nbobj/1048576);
	printf("%d Objets, %d Pts, %d Faces, %d obj autoréférencés, %d obj immobiles\n",nbobj,j,k,l,dtradio);*/
	dtradio=0;
	debtir=nbobj;
	for (i=0; i<NBMAXTIR; i++) vieshot[i]=0;
	if ((obj=realloc(obj, sizeof(objet)*(nbobj+NBMAXTIR+1)))==NULL) { perror("realloc bot"); exit(-1); }
	clearprime();
	for (i=0; i<NBGRAVMAX; i++) debris[i].o=-1;
	for (i=0; i<NBFUMEESOURCE; i++) fumeesourceintens[i]=0;
	copyv(&obj[0].pos,&obj[bot[bmanu].vion].pos);
	copym(&obj[0].rot,&obj[bot[bmanu].vion].rot);
	visubot=bmanu;
	bombidx=0;
	copyv(&posc[0],&vec_zero);
	copyv(&posc[1],&vec_zero);
	copyv(&OldCamDep,&vec_zero);
	if (MonoMode) inittime();
	// effacer les tableaux de bord, les poscams et les charnières
	for (i=0; i<NBBOT; i++) {
		obj[bot[i].vion+viondesc[bot[i].navion].tabbord].aff=0;
		obj[bot[i].vion+nobjet[bot[i].navion].nbpieces-1].aff=0;
		for (j=0; j<viondesc[bot[i].navion].nbcharngearx+viondesc[bot[i].navion].nbcharngeary+3; j++)
			obj[bot[i].vion+j+viondesc[bot[i].navion].nbmoyeux+1].aff=0;
	}
	// VISION
	playsound(VOICEEXTER,TARATATA,1,1,0);
	DT=AccelFactor=0;	// parcequ'au premier passage, on n'a pas encore lut le DT du réseau avant de calculer AccelFactor
	do {
		vector v,u;
		Exploze=0;
		imgcount++;
		if (accel) DT=NORMALDT;
		n=AccelFactor;
		AccelFactor=(float)DT/NORMALDT;
		if (AccelFactor<n-.3) AccelFactor=n-.3;
		else if (AccelFactor>n+.3) AccelFactor=n+.3;
		if (AccelFactor>3) AccelFactor=3;
		else if (AccelFactor<.1) AccelFactor=.1;
		// PJ
		manuel(bmanu);
		// PNJ
		if (!lapause) {
			// calcul les pos du sol
			for (i=0; i<NBBOT; i++) bot[i].zs=obj[bot[i].vion].pos.z-z_ground(obj[bot[i].vion].pos.x,obj[bot[i].vion].pos.y, true);
			NetSend();
			// mettre entre les deux accès réseau tout ce qui ne dépend
			// pas des commandes des playbots !
			for (i=NbHosts; i<NBBOT; i++) robot(i);
			for (i=0; i<NBTANKBOTS; i++) robotvehic(i);
			// vérifie que les playbots ne heurtent rien
			if (!Easy) {
				for (j=0; j<NbHosts; j++) if (bot[j].camp!=-1) {
					for (i=0; i<NBBOT; i++) if (i!=j && bot[i].camp!=-1 && collision(bot[j].vion,bot[i].vion)) break;
					if (i<NBBOT) {
						explose(bot[i].vion,bot[j].vion);
						explose(bot[j].vion,bot[i].vion);
					}
					for (i=0; i<NBZEPS; i++) if (collision(bot[j].vion,zep[i].o)) break;
					if (i<NBZEPS) explose(bot[j].vion,zep[i].o);
				}
			}
			// message d'alerte ?
			if (bot[bmanu].camp!=-1 && bot[bmanu].vionvit.z<-5 && (n=bot[bmanu].vionvit.z*100+bot[bmanu].zs)<0) playsound(VOICEALERT,ALERT,1-n*.0001,1,0);
			// avance les shots
			for (i=debtir; i<nbobj; i++) {
				int oc, fg=0;
				if (!vieshot[i-debtir]) continue;
				vieshot[i-debtir]--;
				// collision ?
				for (oc=map[obj[i].ak].first_obj; oc!=-1; oc=obj[oc].next)
					if (obj[oc].type!=BOMB && collision(i,oc)) { fg=1; break; }
				if (fg) {
					vieshot[i-debtir]=0;
				//	printf("Shot %d blow guts out of %d\n",i,oc);
					hitgun(oc,i);
				}
				copyv(&v, &obj[i].rot.x);
				mulv(&v,100*AccelFactor);
				addv(&obj[i].pos,&v);
				copyv(&v,&vec_g);
				mulv(&v,.005*AccelFactor);
				addv(&obj[i].rot.x,&v);
				renorme(&obj[i].rot.x);
				randomv(&obj[i].rot.y);
				orthov(&obj[i].rot.y,&obj[i].rot.x);
				prodvect(&obj[i].rot.x,&obj[i].rot.y,&obj[i].rot.z);
				controlepos(i);
				if (vieshot[i-debtir]==0 || obj[i].pos.z<z_ground(obj[i].pos.x,obj[i].pos.y, true)) {
					obj[i].aff=0;	// pour qu'il soit plus affiché
#ifndef DEMO
					if (i==nbobj-1) do {
						nbobj--; nbtir--;
						if (obj[nbobj].next!=-1) obj[obj[nbobj].next].prec=obj[nbobj].prec;
						if (obj[nbobj].prec!=-1) obj[obj[nbobj].prec].next=obj[nbobj].next;
						else map[obj[nbobj].ak].first_obj = obj[nbobj].next;
						// comme ca on est sur que calcposa va pas venir
						// mettre ce tir mort dans un autre ak s'il est à
						// cheval sur une frontiere, puisqu'il ne bouclera
						// plus jusqu'ici
					} while (obj[nbobj-1].aff==0 && nbobj>debtir);
#endif
				}
			}
			// déplace les bombes
			for (i=0; i<bombidx; i++) {
				j=bombe[i].o;
				if (j!=-1) {
					int oc, fg=0;
					copyv(&v,&vec_g);
				//	mulv(&v,G_FACTOR*AccelFactor);
					addv(&bombe[i].vit,&v);
					mulv(&bombe[i].vit,.999);//pow(.999,AccelFactor));
					copyv(&v,&bombe[i].vit);
					mulv(&v,AccelFactor);
					addv(&obj[j].pos,&v);//bombe[i].vit);
					controlepos(j);
					// collision ?
					for (oc=map[obj[j].ak].first_obj; oc!=-1; oc=obj[oc].next)
						if (obj[oc].type!=TIR && obj[oc].type!=CAMERA && obj[oc].type!=DECO && (oc<bot[bombe[i].b].vion || oc>=bot[bombe[i].b].vion+nobjet[bot[bombe[i].b].navion].nbpieces) && collision(j,oc)) { explose(oc,j); fg=1; break; }
					if (fg || obj[j].pos.z<z_ground(obj[j].pos.x,obj[j].pos.y, true)) {
						if (!fg) {
							float np;
							copyv(&p,&obj[j].pos);
							subv(&p,&obj[0].pos);
							np=renorme(&p);
							playsound(VOICEEXTER,EXPLOZ2,1+(drand48()-.5)*.08,MIN(2.,1./(1+np*np*1e-6)),128*scalaire(&p,&obj[0].rot.x));
						}
						obj[j].objref=bot[bombe[i].b].babase;
						copyv(&obj[j].pos,&vec_zero);
						copym(&obj[j].rot,&mat_id);
						bombe[i].o=-1;
						while (bombe[bombidx-1].o==-1) bombidx--;
					}
				}
			}
			// avance les débris
			for (i=0; i<NBGRAVMAX; i++) {
				if (debris[i].o!=-1) {
					double zs;
					float c1,c2,s1,s2;
					copyv(&v,&debris[i].vit);
					mulv(&v,AccelFactor);
					addv(&obj[debris[i].o].pos,&v);
					c1=cos(debris[i].a1);
					c2=cos(debris[i].a2);
					s1=sin(debris[i].a1);
					s2=sin(debris[i].a2);
					obj[debris[i].o].rot.x.x=c1*c2;
					obj[debris[i].o].rot.x.y=s1*c2;
					obj[debris[i].o].rot.x.z=s2;
					obj[debris[i].o].rot.y.x=-s1;
					obj[debris[i].o].rot.y.y=c1;
					obj[debris[i].o].rot.y.z=0;
					obj[debris[i].o].rot.z.x=-c1*s2;
					obj[debris[i].o].rot.z.y=-s1*s2;
					obj[debris[i].o].rot.z.z=c2;
					debris[i].a1+=debris[i].ai1*AccelFactor;
					debris[i].a2+=debris[i].ai2*AccelFactor;
					controlepos(debris[i].o);
					mulv(&debris[i].vit,pow(.999,AccelFactor));
					debris[i].vit.z-=G_FACTOR;
					zs=z_ground(obj[debris[i].o].pos.x,obj[debris[i].o].pos.y, true);
					if (obj[debris[i].o].pos.z<zs) {
						obj[debris[i].o].pos.z=zs;
						debris[i].vit.z=-debris[i].vit.z;
						mulv(&debris[i].vit,.5);
						if (debris[i].vit.z<1) {
						//	copym(&obj[debris[i].o].rot,&mat_id);
							randomhm(&obj[debris[i].o].rot);
							debris[i].o=-1;
						}

					}
				}
			}
			// avance la fumee
			for (fumeedispo=0, i=0; i<NBMAXFUMEE; i++) {
				if (rayonfumee[i]) {
					uchar rlim;
					randomv(&v);
					mulv(&v,rayonfumee[i]);
					v.z+=rayonfumee[i]>>1;
					switch(typefumee[i]) {
					case 0: rlim=90; break;
					default: rlim=6; break;
					}
					if (++rayonfumee[i]>rlim) rayonfumee[i]=0;
					else {
						mulv(&v,AccelFactor);
						addv(&obj[firstfumee+i].pos,&v);
						controlepos(firstfumee+i);
					}
				} else fumeedispo=i;
			}
			// fait fumer
			for (i=0; i<NBBOT; i++) {
				if (bot[i].burning) {
					bot[i].burning--;
					if (!(bot[i].burning&3)) {
						for (; rayonfumee[fumeedispo] && fumeedispo>=0; fumeedispo--);
						if (fumeedispo>=0) {
							rayonfumee[fumeedispo]=1;
							typefumee[fumeedispo]=0;	// type noir
							copyv(&obj[firstfumee+fumeedispo].pos,&obj[bot[i].vion].pos);
							controlepos(firstfumee+fumeedispo);
						}
					}
				}
			}
			for (i=0; i<NBFUMEESOURCE; i++) {
				if (fumeesourceintens[i]>0) {
					fumeesourceintens[i]--;
					if (!(fumeesourceintens[i]&1)) {
						for (; rayonfumee[fumeedispo] && fumeedispo>=0; fumeedispo--);
						if (fumeedispo>=0) {
							rayonfumee[fumeedispo]=1;
							copyv(&obj[firstfumee+fumeedispo].pos,&obj[fumeesource[i]].pos);
							controlepos(firstfumee+fumeedispo);
						}
					}
				}
			}
			// fait avancer les voitures
			for (i=0; i<NBVOITURES; i++) {
				int dist;
				if (obj[voiture[i].o].type==DECO) continue;
				subv3(&route[voiture[i].r].i,&obj[voiture[i].o].pos,&u);
				dist=fabs(u.x)+fabs(u.y);
				if (dist>voiture[i].dist) {
					if (voiture[i].r<2 || voiture[i].r>routeidx-3 || route[voiture[i].r+voiture[i].sens].ak==-1) voiture[i].sens=-voiture[i].sens;
					if (route[voiture[i].r+voiture[i].sens].ak!=-1) {
						subv3(&route[voiture[i].r+voiture[i].sens].i,&route[voiture[i].r].i,&obj[voiture[i].o].rot.x);
						renorme(&obj[voiture[i].o].rot.x);
						orthov(&obj[voiture[i].o].rot.z,&obj[voiture[i].o].rot.x);
						renorme(&obj[voiture[i].o].rot.z);
						prodvect(&obj[voiture[i].o].rot.z,&obj[voiture[i].o].rot.x,&obj[voiture[i].o].rot.y);
						voiture[i].r+=voiture[i].sens;
						dist=MAXINT;
					} else voiture[i].vit=0;
				}
				voiture[i].dist=dist;
				copyv(&v,&obj[voiture[i].o].rot.x);
				mulv(&v,AccelFactor*voiture[i].vit);
				addv(&obj[voiture[i].o].pos,&v);
				controlepos(voiture[i].o);
				for (j=voiture[i].o+1; j<voiture[i+1].o; j++) calcposrigide(j);
			}
			// messages radio ?
			if (msgactutime) msgactutime--;
			else if (initradio || !dtradio--) {
				if (!SpaceInvaders && initradio) {
					campactu=camp;
					strcpy(msgactu,scenar[campactu][4-initradio][lang]);
					initradio--;
					playsound(VOICEGEAR,MESSAGE,1,1,0);
					msgactutime=40;
				} else {
					newprime();
					if (campactu==bot[visubot].camp) playsound(VOICEGEAR,MESSAGE,1,1,0);
					dtradio=10+drand48()*100;
					if (campactu==0) {
						dtradio+=10000;
					}
				}
			}
			if (NetRead()==-1) {
				printf("NET ERROR. Exiting...\n");
				goto fin;
			}
			for (i=0; i<NBBOT; i++) control(i);
			for (i=0; i<NBTANKBOTS; i++) controlvehic(i);
			for (i=0; i<NBZEPS; i++) controlzep(i);

			// fait tourner les moulins
			AngleMoulin+=drand48()*.2*AccelFactor;
			m.x.x=1; m.x.y=0; m.x.z=0;
			m.y.x=0; m.y.y=cos(AngleMoulin); m.y.z=sin(AngleMoulin);
			m.z.x=0; m.z.y=-sin(AngleMoulin); m.z.z=cos(AngleMoulin);
			for (i=DebMoulins+1; i<FinMoulins; i+=10){
				if (obj[i].type!=DECO) {
					calcposarti(i,&m);
					for (j=i+1; j<i+5; j++) calcposrigide(j);
				}
			}
			if (!accel || imgcount>64) {
				if (accel) imgcount-=64;
				// où est la caméra ?
				if (visu==3) {
					if (!visubomb || obj[visubomb].objref!=-1) {
						for (i=bot[visubot].vion; i<bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces && (obj[i].objref!=-1 || obj[i].type!=BOMB); i++);
						if (i<bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces) visubomb=i; else { visubomb=0; visu=4; }
					}
				}
				if (visu==4 /*&& !Gruge*/) visu=5;
				if (visu==7) {
					if (visubot!=bmanu) visu=0;
					else {
						if (DogBot==bmanu || bot[DogBot].camp==-1) NextDogBot();
						if (DogBot!=bmanu && bot[DogBot].camp!=-1) {
							copyv(&DogBotDir,&obj[bot[DogBot].vion].pos);
							subv(&DogBotDir,&obj[bot[bmanu].vion].pos);
							DogBotDist=renorme(&DogBotDir);
							if (DogBotDist>DOGDISTMAX) NextDogBot();
							if (DogBotDist>DOGDISTMAX) visu=0;
						} else visu=0;
					}
				}
				if (visu==0 || visu==7) {	// afficher ou effacer la tete et la tab de bord
					for (i=0; i<viondesc[bot[visubot].navion].nbpiecestete; i++)
						obj[bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces-2-i].aff=0;
					obj[bot[visubot].vion+viondesc[bot[visubot].navion].tabbord].aff=1;
				} else {
					for (i=0; i<viondesc[bot[visubot].navion].nbpiecestete; i++)
						obj[bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces-2-i].aff=1;
					obj[bot[visubot].vion+viondesc[bot[visubot].navion].tabbord].aff=0;
				}
				switch (visu) {
				case 7:
				case 0:
					{ matrix ct;
					copyv(&posc[0],&posc[1]);
					copyv(&posc[1],&bot[visubot].vionvit);
					copyv(&obj[0].pos,&obj[bot[visubot].vion+nobjet[bot[visubot].navion].nbpieces-1].pos);
					if (visu!=7 || avancevisu || tournevisu) {
						copyv(&ct.x,&obj[bot[visubot].vion].rot.y);
						neg(&ct.x);
						copyv(&ct.y,&obj[bot[visubot].vion].rot.z);
						neg(&ct.y);
						copyv(&ct.z,&obj[bot[visubot].vion].rot.x);
					} else {
						copyv(&ct.z,&DogBotDir);
						copyv(&ct.y,&obj[bot[visubot].vion].rot.z);
						neg(&ct.y);
						orthov(&ct.y,&ct.z);
						renorme(&ct.y);
						prodvect(&ct.y,&ct.z,&ct.x);
					}
					if (!avancevisu) {	// Tu me fait tourner... la tete...
						matrix m;
						double ctt,st,cf,sf;
						ctt=cos(visuteta);
						st=sin(visuteta);
						cf=cos(visuphi);
						sf=sin(visuphi);
						m.x.x=cf;		m.y.x=sf*st;		m.z.x=-sf*ctt;
						m.x.y=0;			m.y.y=ctt;			m.z.y=st;
						m.x.z=sf;		m.y.z=-st*cf;		m.z.z=cf*ctt;
						mulm(&ct,&m);
						copyv(&v,&vec_zero);
					} else {
						copyv(&v,&ct.z);
						mulv(&v,2.1);
						addv(&obj[0].pos,&v);
						copyv(&v,&ct.y);
						mulv(&v,5.2);
					}
					copyv(&u,&posc[0]);
					subv(&u,&posc[1]);
					mulv(&u,3);
					addv(&v,&u);
					subv(&v,&OldCamDep);
					mulv(&v,.4);
					addv(&v,&OldCamDep);
					copyv(&OldCamDep,&v);
					if (accel) mulv(&v,.002);
					if (norme2(&v)<400) addv(&obj[0].pos,&v);
					addv(&obj[0].rot.x,&ct.x);
					addv(&obj[0].rot.y,&ct.y);
					renorme(&obj[0].rot.x);
					orthov(&obj[0].rot.y,&obj[0].rot.x);
					renorme(&obj[0].rot.y);
					prodvect(&obj[0].rot.x,&obj[0].rot.y,&obj[0].rot.z);
					}
					break;
				case 1:
					obj[0].rot.y.x=0; obj[0].rot.y.y=0; obj[0].rot.y.z=-1;
					obj[0].rot.x.x=cos(angvisu1); obj[0].rot.x.y=sin(angvisu1); obj[0].rot.x.z=0;
					obj[0].rot.z.x=-sin(angvisu1); obj[0].rot.z.y=cos(angvisu1); obj[0].rot.z.z=0;
					copyv(&obj[0].pos,&obj[0].rot.z);
					mulv(&obj[0].pos,-loinvisu);
					addv(&obj[0].pos,&obj[bot[visubot].vion].pos);
					angvisu1+=0.04;
					if (obj[0].pos.z<(n=30+z_ground(obj[0].pos.x,obj[0].pos.y, true))) obj[0].pos.z=n;
					break;
				case 2:
					copym(&obj[0].rot,&mat_id);
					neg(&obj[0].rot.z); neg(&obj[0].rot.y);
					copyv(&obj[0].pos,&obj[bot[visubot].vion].pos);
					obj[0].pos.z+=loinvisu;
					break;
				case 3:
					obj[0].rot.y.x=0; obj[0].rot.y.y=0; obj[0].rot.y.z=-1;
					obj[0].rot.x.x=cos(angvisu1); obj[0].rot.x.y=sin(angvisu1); obj[0].rot.x.z=0;
					obj[0].rot.z.x=-sin(angvisu1); obj[0].rot.z.y=cos(angvisu1); obj[0].rot.z.z=0;
					copyv(&obj[0].pos,&obj[0].rot.z);
					mulv(&obj[0].pos,-loinvisu);
					addv(&obj[0].pos,&obj[visubomb].pos);
					angvisu1+=0.03;
					break;
				case 4:
					obj[0].rot.y.x=0; obj[0].rot.y.y=0; obj[0].rot.y.z=-1;
					obj[0].rot.x.x=cos(angvisu1); obj[0].rot.x.y=sin(angvisu1); obj[0].rot.x.z=0;
					obj[0].rot.z.x=-sin(angvisu1); obj[0].rot.z.y=cos(angvisu1); obj[0].rot.z.z=0;
					copyv(&obj[0].pos,&obj[0].rot.z);
					mulv(&obj[0].pos,-loinvisu);
					addv(&obj[0].pos,&obj[visuobj].pos);
					angvisu1+=0.03;
					break;
				case 5:
					copyv(&obj[0].pos,&obj[bot[visubot].vion].pos);
					copyv(&obj[0].rot.x,&obj[bot[visubot].vion].rot.y);
					neg(&obj[0].rot.x);
					copyv(&obj[0].rot.y,&obj[bot[visubot].vion].rot.z);
					neg(&obj[0].rot.y);
					copyv(&obj[0].rot.z,&obj[bot[visubot].vion].rot.x);
					copyv(&p,&obj[0].rot.z);
					mulv(&p,-(loinvisu-80));
					addv(&obj[0].pos,&p);
					copyv(&p,&bot[visubot].vionvit);
					mulv(&p,-3);
					addv(&obj[0].pos,&p);
					if (obj[0].pos.z<(n=30+z_ground(obj[0].pos.x,obj[0].pos.y, true))) obj[0].pos.z=n;
					break;
				case 6:
					subv3(&obj[bot[visubot].vion].pos,&obj[0].pos,&obj[0].rot.z);
					renorme(&obj[0].rot.z);
					obj[0].rot.y.x=obj[0].rot.y.y=0;
					obj[0].rot.y.z=-1;
					orthov(&obj[0].rot.y,&obj[0].rot.z);
					renorme(&obj[0].rot.y);
					prodvect(&obj[0].rot.y,&obj[0].rot.z,&obj[0].rot.x);
					break;
				}
				// RENDU
				// La lumière vient d'où ?
				copym(&Light,&LightSol);
				if (Exploze) {
					subv3(&obj[0].pos,&ExplozePos,&u);
					if (renorme(&u)<ECHELLE) {
						copyv(&Light.z,&u);
						Light.x.x=u.y;
						Light.x.y=u.z;
						Light.x.z=u.x;
						orthov(&Light.x,&Light.z);
						renorme(&Light.x);
						prodvect(&Light.z,&Light.x,&Light.y);
					}
				}
				animsoleil();
				if (mapmode) {
					rendumapbg();
					rendumap();
				} else {
					background();
					affsoleil(&Light.z);
					mulmtv(&obj[bot[visubot].vion].rot,&Light.z,&v);
					lx=-127*v.y; ly=-127*v.z; lz=50*v.x+77;
					if (viondesc[bot[visubot].navion].oldtb) tbback=tbback1;
					else tbback=tbback2;
					drawtbback();
					drawtbcadrans(visubot);
					animate_water();
					draw_ground_and_objects();
					if (!Dark) {
						double i;
						uchar u;
						if ((i=scalaire(&obj[0].rot.z,&Light.z))<-.9) {
							u=(exp(-i-.9)-1)*2200;
							MMXAddSatInt((int*)videobuffer,(u<<16)+(u<<8)+u,SX*SY);
						}
					}
				}
				if (visu==7 && bot[bmanu].camp!=-1) cercle(0,0,10,colcamp[(int)bot[bmanu].camp]);
				plotmouse(_DX*bot[visubot].xctl,_DY*bot[visubot].yctl);
/*				if (bot[visubot].voltige) pnum(bot[visubot].voltige,SX-50,20,0xFFFFFF,1);
				else pnum(bot[visubot].manoeuvre,SX-50,20,0xFFFF00,1);
				pnum(bot[visubot].vitlin,SX-50,30,0xFFFFFF,1);
				pnum(norme(&bot[visubot].vionvit),SX-50,40,0xFFFF,1);
				pnum(bot[visubot].zs,SX-50,50,0xFFFF,1);
				pnum(bot[visubot].cibt,SX-50,60,0xFFFFFF,1);
				pnum(bot[visubot].gunned,SX-50,70,0xFFFFFF,1);*/
				// AFFICHAE DIGITAL MODE FACILE
				if (Easy) {
					pnum(bot[bmanu].vionvit.z,10,10,0xAFDF10,1);
					pnum(bot[bmanu].vitlin,10,20,0xFFFFFF,1);
					pnum(norme(&bot[bmanu].vionvit),10,30,0xFFFF,1);
					pnum(bot[bmanu].zs,10,40,0xFF00FF,1);
					if (bot[bmanu].but.gear) pword("gear",10,60,0xD0D0D0);
					if (bot[bmanu].but.flap) pword("flaps",10,70,0xD0D0D0);
					if (bot[bmanu].but.frein) pword("brakes",10,80,0xD0D0D0);
					if (autopilot) pword("auto",10,90,0xD0D0D0);
				}
				if (accel) pstr("ACCELERATED MODE",_DY/3,0xFFFFFF);
				if (quitte==1) pstr("Quit ? Yes/No",_DY/2-8,0xFFFFFF);
				if (msgactutime && bot[visubot].camp==campactu) pstr(msgactu,10,0xFFFF00);
				if (visu==7 && bot[DogBot].camp!=-1) {
					char vn[100];
					strcpy(vn,viondesc[bot[DogBot].navion].name);
					if (DogBot<NbHosts) {
						strcat(vn," (");
						strcat(vn,&(playbotname[DogBot])[0]);
						strcat(vn,")");
					}
					pstr(vn,SY-10, colcamp[(int)bot[DogBot].camp]);
				}
				if (bot[bmanu].gold-2000>maxgold) {
					maxgold=bot[bmanu].gold-2000;
					if (maxrank<20) highscore[maxrank].score=maxgold;
					while (maxrank>0 && highscore[maxrank-1].score<maxgold) {
						maxrank--;
						if (maxrank<19) memcpy(&highscore[maxrank+1],&highscore[maxrank],sizeof(HS_s));
						highscore[maxrank].score=maxgold;
						strcpy(highscore[maxrank].name,&(playbotname[bmanu])[0]);
					}
				}
				if (bot[bmanu].gold>oldgold) {
					if (!caissetot && caisse>0) caisse+=bot[bmanu].gold-oldgold;
					else caisse=bot[bmanu].gold-oldgold;
					dtcaisse=20;
					caissetot=0;
				} else if (oldgold>bot[bmanu].gold) {
					if (!caissetot && caisse<0) caisse+=bot[bmanu].gold-oldgold;
					else caisse=bot[bmanu].gold-oldgold;
					dtcaisse=20;
					caissetot=0;
				}
				if (dtcaisse) {
					pbignum(caisse,_DX,SY/3,2,caissetot,1);
					dtcaisse--;
					if (!dtcaisse) {
						if (!caissetot) {
							caisse=bot[bmanu].gold;
							dtcaisse=30;
							caissetot=1;
							playsound(VOICEGEAR,MESSAGE,1.4,1,0);
						} else {
							caissetot=0;
							caisse=0;
						}
					}
				}
				oldgold=bot[bmanu].gold;

				if (AfficheHS) {
					int y=(SY-(20+2)*9)>>1;
					pstr("LE TOP 20 DE LA FRIME",y,0xFFFFFF);
					for (i=0; i<20; i++) {
						char fonom[36];
						sprintf(fonom,"%d. %s",highscore[i].score,highscore[i].name);
						pstr(fonom,y+9*(2+i),i==maxrank?0xFFFF1F:0xEFD018);
					}
				}
				plotcursor(xmouse,ymouse);
		/*		p1.x=_DX+   0*cos(bolop)-  -50*sin(bolop);
				p1.y=_DY+   0*sin(bolop)+  -50*cos(bolop);
				p2.x=_DX+ -90*cos(bolop)-   160*sin(bolop);
				p2.y=_DY+ -90*sin(bolop)+   160*cos(bolop);
				p3.x=_DX+ -70*cos(bolop)-   80*sin(bolop);
				p3.y=_DY+ -70*sin(bolop)+   80*cos(bolop);
				p4.x=_DX+ -5*cos(bolop)-  -80*sin(bolop);
				p4.y=_DY+ -5*sin(bolop)+  -80*cos(bolop);
				MMXSaveFPU();
				polygouro(&p1,&p2,&p3);
				polygouro(&p4,&p1,&p3);
				plot(p1.x-_DX,p1.y-_DY,0xFFFFFF);
				plot(p2.x-_DX,p2.y-_DY,0xFFFFFF);
				plot(p3.x-_DX,p3.y-_DY,0xFFFFFF);
				plot(p2.x-_DX,p2.y-_DY,0xFFFFFF);
				MMXRestoreFPU();
				bolop+=.02;*/
				buffer2video();
			}
		}
	} while (quitte<2);
	// FIN
fin:
//	XFreeFont(disp,xfont);
//	XAutoRepeatOn(disp);	// Ceci concerne tous les programmes, donc
//	aussi d'autres clients tournant sur la meme machine... ce qui
//	théoriquement n'arrive que chez moi pour les tests.
	signal(SIGALRM,SIG_DFL);
	exitsound();
	system("xset r on");	// pis aller
	// sauver les highscore
	if (!Easy && !ViewAll && viondesc[monvion-1].prix<=viondesc[0].prix && (file=fopen(".highscores","w+"))!=NULL) {
		fwrite(&highscore,sizeof(HS_s),20,file);
		fclose(file);
	}
	{
		// affiche les highscores
		char *rank[9][2]={
			{ "affamé", "famished" },
			{ "assisté", "needy" },
			{ "consommateur", "consumer" },
			{ "petit épargnant", "saving" },
			{ "entrepreneur", "contractor" },
			{ "investisseur", "investor" },
			{ "actionnaire", "share holder" },
			{ "ministre", "cabinet secretary" },
			{ "PDG", "managing director" }
		};
		printf("\n-------------------------------\n\n    Best peace soldiers :\n\n");
		for (i=0; i<20; i++) {
			printf("    %2d) $%5d - %s\n",i,highscore[i].score,highscore[i].name);
		}
		i=(maxgold>2000)+(maxgold>4000)+(maxgold>6000)+(maxgold>8000)+(maxgold>10000)+(maxgold>12000)+(maxgold>14000)+(maxgold>16000);
		printf("\n    Your score : %d\n    %s %s.\n\n",maxgold,lang?"You retreat as a":"Vous vous retirez en tant que",rank[i][lang]);
	}
	exit(0);
}
