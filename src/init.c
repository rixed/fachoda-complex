#include <math.h>
#include "3d.h"
char *tankname="Rug-Warrior";
int bosse(int a) {
	if (a) return abs((int)map[a]-map[a+1])+abs((int)map[a]-map[a+WMAP])+abs((int)map[a]-map[a+1+WMAP]);
	else return MAXINT;
}
void randomhm(matrix *m) {
	float a=randK()*M_PI*2;
	copym(m,&mat_id);
	m->x.x=cos(a);
	m->x.y=sin(a);
	m->y.x=-m->x.y;
	m->y.y=m->x.x;
}
void posem(matrix *m, vector *p) {	// tourne légèrement la matrice pour la "poser" sur le sol
	m->x.z=zsol(p->x+m->x.x,p->y+m->x.y)-zsol(p->x,p->y);
	renorme(&m->x);
	m->y.x=-m->x.y;
	m->y.y=m->x.x;
	m->y.z=zsol(p->x+m->y.x,p->y+m->y.y)-zsol(p->x,p->y);
	renorme(&m->y);
	prodvect(&m->x,&m->y,&m->z);
}
int randomvroute(vector *v) {
	int i;
	do i=2+randK()*(routeidx-4); while(route[i+1].ak==-1 || route[i].ak==-1);
	v->x=route[i].i.x;
	v->y=route[i].i.y;
	v->z=route[i].i.z;
	return i;
}
int bossep(vector *p) { return bosse(akpos(p)); }
void addbabase(int c) {
	int x,y;
	int x1[4]={WMAP/5.,3.*WMAP/5.,WMAP/5.,3.*WMAP/5.};
	int y1[4]={WMAP/5.,WMAP/5.,3.*WMAP/5.,3.*WMAP/5.};
	int a[3]={0,0,0};
	matrix m;
	vector p,pp;
	for (y=y1[c]; y<y1[c]+3.*WMAP/10.; y+=5)
		for (x=x1[c]; x<x1[c]+3.*WMAP/10.; x+=5) {
			int aa=x+(y<<NWMAP);
			if (map[aa]>50 && map[aa]<120) {
				if (!a[0] || bosse(aa)<bosse(a[0])) {
					a[2]=a[1]; a[1]=a[0]; a[0]=aa;
				} else if (!a[1] || bosse(aa)<bosse(a[1])) {
					a[2]=a[1]; a[1]=aa;
				} else if (!a[2] || bosse(aa)<bosse(a[2])) a[2]=aa;
			}
		}
	for (x=0; x<3; x++) {
		int yb=a[x]>>NWMAP;
		int xb=a[x]-(yb<<NWMAP);
		p.x=(xb-(WMAP>>1))*ECHELLE+ECHELLE/2;
		p.y=(yb-(WMAP>>1))*ECHELLE+ECHELLE/2;
		p.z=0;
		mapmap[a[x]-1]=0;
		mapmap[a[x]]=0;
		mapmap[a[x]+1]=0;
		mapmap[a[x]+2]=0;
		mapmap[a[x]+3]=1;
		mapmap[a[x]-1]=1;
		mapmap[a[x]-WMAP]=0;
		mapmap[a[x]+WMAP]=0;
		mapmap[a[x]+WMAP+1]=0;
		mapmap[a[x]-WMAP+1]=0;
		mapmap[a[x]+WMAP-1]=0;
		mapmap[a[x]-WMAP-1]=0;
		mapmap[a[x]+WMAP+2]=1;
		mapmap[a[x]-WMAP+2]=1;
		map[a[x]-1]=map[a[x]];
		map[a[x]+1]=map[a[x]];
		map[a[x]+2]=map[a[x]];
		map[a[x]-WMAP]=map[a[x]];
		map[a[x]-WMAP-1]=map[a[x]];
		map[a[x]-WMAP+1]=map[a[x]];
		map[a[x]-WMAP+2]=map[a[x]];
		map[a[x]+WMAP]=map[a[x]];
		map[a[x]+WMAP-1]=map[a[x]];
		map[a[x]+WMAP+1]=map[a[x]];
		map[a[x]+WMAP+2]=map[a[x]];
	}
	for (x=0; x<3; x++) {
		int yb=a[x]>>NWMAP;
		int xb=a[x]-(yb<<NWMAP);
		p.x=(xb-(WMAP>>1))*ECHELLE+ECHELLE/2;
		p.y=(yb-(WMAP>>1))*ECHELLE+ECHELLE/2;
		p.z=0;
		babaseo[0][x][c]=addnobjet(NBNAVIONS, &p, &mat_id, 1);	// la piste
	
		copyv(&pp,&p);
		pp.x+=(randK()-.5)*ECHELLE*.4;
		pp.y+=(randK()+.5)*ECHELLE*.2;
		pp.z=85;
		randomhm(&m);
		addnobjet(NBNAVIONS+1, &pp, &m, 1);	// une tour de controle
		for (y=0; y<3; y++) {
			pp.x+=(randK()-.5)*ECHELLE*.2;
			pp.y+=(randK()-.5)*ECHELLE*.2;
			pp.z=15;
			randomhm(&m);
			addnobjet(NBNAVIONS+NBBASES+NBMAISONS+randK()*3, &pp, &m, 1);	// des vehic
		}
		babaseo[1][x][c]=nbobj;
		for (y=0; y<NBNAVIONS; y++) {
			matrix mp = {
				{ cos(.3), 0, sin(.3) },
				{ 0,1,0 },
				{ -sin(.3),0,cos(.3) }
			};
			copyv(&pp,&p);
			pp.x-=600;
			pp.y+=(y-NBNAVIONS/2)*200;
			pp.z=20;
			addnobjet(y, &pp, y!=2?&mp:&mat_id, 1);
		}
	//	printf("Add Babase #%d, C#%d, at %f,%f\n",x,c,p.x,p.y);
	}
}
void randomvferme(vector *p) {
	int c,i,ok;
	do {
		ok=1;
		randomv(p);
		mulv(p,ECHELLE*(WMAP-WMAP/4));
		p->z=zsol(p->x,p->y);
		for (c=0; c<4; c++) for (i=0; i<3; i++) {
			vector pp;
			copyv(&pp,&obj[babaseo[0][i][c]].pos);
			subv(&pp,p);
			if (norme(&pp)<ECHELLE*10) ok=0;
		}
	} while (map[akpos(p)]<80 || map[akpos(p)]>150 || !ok);
}
int collisionpoint(vector *pp, int k, int mo) {
	vector u;
	subv3(pp,&obj[k].pos,&u);
	return norme(&u)<=mod[mo].rayoncollision+mod[obj[k].model].rayoncollision;
}
void affjauge(float j) {
	static float jauge=0;
	float nj=jauge+j;
	static int x=10;
	int nx,y,xx;
	nx=10+(int)(nj*(SX-20.));
	if (nx>x) {
		MMXSaveFPU();
		for (y=_DY-(SY>>3); y<_DY+(SY>>3); y++)
			for (xx=x; xx<nx; xx++)
				//*(int*)&videobuffer[y*SX+xx]=0x3060A0;
				MMXAddSatC((int*)&videobuffer[y*SX+xx],0x001080);
		MMXRestoreFPU();
		buffer2video();
		x=nx;
	}
	jauge=nj;
}

void initworld() {
	int i,j,k; vector p; matrix m;
	bombe=(bombe_s*)malloc(NBBOT*4*sizeof(bombe_s));
	vehic=(vehic_s*)malloc(NBTANKBOTS*sizeof(vehic_s));
	initmap();
	obj=(objet*)malloc(sizeof(objet)*50000);
	// caméra
	obj[0].objref=-1;
	obj[0].type=CAMERA;
	obj[0].distance=-1;
	obj[0].prec=-1; obj[0].next=-1;
	fomap[0]=0;
	obj[0].ak=0;
	nbobj=1;
	// babases
	printf("Adding bases\n");
	for (j=0; j<4; j++) addbabase(j);
	printf("end bases\n");
	// zeppelins
	if ((zep=malloc(sizeof(zep_s)*NBZEPS))==NULL) {
		perror("malloc zep"); exit(-1);
	}
	for (i=0; i<NBZEPS; i++) {
		p.x=(randK()-.5)*(WMAP<<NECHELLE)*0.8;
		p.y=(randK()-.5)*(WMAP<<NECHELLE)*0.8;
		p.z=5000+zsolraz(p.x,p.y);
		zep[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+NBDECOS,&p, &mat_id, 0);
		copyv(&zep[i].nav,&p);
		zep[i].angx=zep[i].angy=zep[i].angz=zep[i].anghel=zep[i].vit=0;
		for (j=0; j<6; j++) zep[i].cib[j]=-1;
	}
	// installations au sol
	// des villages de pauvres inocents pour souffrir
	for (i=0; i<NBVILLAGES; i++) {
		int nbm=randK()*20+5;
		int ok,k;
		vector pp;
		int bos=MAXINT, a;
		do {
			for (j=0; j<5; j++) {
				int b;
				randomvferme(&pp);
				if (map[akpos(&pp)]<160 && (b=bossep(&pp))<bos) {
					bos=b;
					copyv(&p,&pp);
				}
			}
			for (j=0; j<i; j++)
				if (fabs(village[j].p.x-p.x)+fabs(village[j].p.y-p.y)<ECHELLE*5) break;
		} while (j<i);
		mapmap[a=akpos(&p)]=0;
		mapmap[a-1]=0;
		mapmap[a+1]=0;
		mapmap[a+WMAP]=0;
		mapmap[a-WMAP]=0;
		mapmap[a+WMAP+1]=0;
		mapmap[a-WMAP+1]=0;
		mapmap[a+WMAP-1]=0;
		mapmap[a-WMAP-1]=0;
		p.z=zsol(p.x,p.y);;
		copyv(&village[i].p,&p);
		village[i].o1=nbobj;
		randomv(&pp);	// une église
		mulv(&pp,100);
		addv(&pp,&p);
		posem(&m,&pp);
		pp.z=40;
		addnobjet(NBNAVIONS+NBBASES+1, &pp, &mat_id, 1);
		for (j=0; j<nbm; j++) {	// des maisons
			matrix m;
			randomhm(&m);
			do {
				ok=1;
				randomv(&pp);
				mulv(&pp,ECHELLE/2);
				addv(&pp,&p);
				posem(&m,&pp);
				pp.z=48;
				for (k=village[i].o1; k<nbobj; k++) if (collisionpoint(&pp,k,NBNAVIONS+NBBASES+0)) {ok=0; break;}
			} while (!ok);
			addnobjet(NBNAVIONS+NBBASES+0, &pp, &m, 1);
		}
		village[i].o2=nbobj;
		village[i].nom=nomvillage[i];
		for (j=0; j<nbm>>1; j++) {	// des reverberes
			matrix m;
			randomhm(&m);
			do {
				ok=1;
				randomv(&pp);
				mulv(&pp,ECHELLE/2);
				addv(&pp,&p);
				posem(&m,&pp);
				pp.z=34;
				for (k=village[i].o1; k<nbobj; k++) if (collisionpoint(&pp,k,NBNAVIONS+NBBASES+5)) {ok=0; break;}
			} while (!ok);
			addnobjet(NBNAVIONS+NBBASES+5, &pp, &m, 1);
		}
	}
	// routes
	initroute();
//	printf("Drawing motorways...\n");
	for (i=0; i<NBVILLAGES-1; i++) for (j=i+1; j<NBVILLAGES; j++) {
//		int d=routeidx;
		int k;
		float cp;
		vector v;
		copyv(&v,&village[i].p);
		subv(&v,&village[j].p);
		if (norme(&v)<ECHELLE*5) continue;
		if (v.x==0 && v.y==0) continue;
		cp=cap(v.x,v.y);
		for (k=i+1; k<j; k++) {
			float dc;
			copyv(&v,&village[i].p);
			subv(&v,&village[k].p);
			if (norme(&v)<ECHELLE*5) continue;
			if (v.x==0 && v.y==0) continue;
			dc=cap(v.x,v.y)-cp;
			if (dc<-M_PI) dc+=2*M_PI;
			else if (dc>M_PI) dc-=2*M_PI;
			if (fabs(dc)<M_PI/4) break;
		}
		if (k==j) {
			prospectroute(&village[i].p,&village[j].p);
//			if (routeidx-d) printf("Motorway between %s to %s, %d elmts long\n",village[i].nom,village[j].nom,routeidx-d);
		}
		affjauge(.75/(1.5*((NBVILLAGES+1)*NBVILLAGES)));
	}
	EndMotorways=routeidx;
//	printf("Drawing roads around cities...\n");
	for (i=0; i<NBVILLAGES; i++) {
		int nbr=randK()*5+5;	// prop à la taille de la ville
		int r;
		for (r=0; r<nbr; r++) {
//			int d=routeidx;
			vector dest;
			randomv(&dest);
			mulv(&dest,ECHELLE*(5+10*randK()));	// prop à la taille de la ville
			addv(&dest,&village[i].p);
			dest.z=zsolraz(dest.x,dest.y);
			prospectroute(&village[i].p,&dest);
//			if (routeidx-d) printf("Road toward %s, %d elmts long\n",village[i].nom,routeidx-d);
			affjauge(.75/(3.*7.5*NBVILLAGES));
		}
	}
	EndRoads=routeidx;
//	printf("Drawing footpaths...\n");
	for (i=0; i<150; i++) {
		int ri=EndMotorways+randK()*(routeidx-EndMotorways-1);
		vector dest,v;
		if (route[ri].ak!=-1 && route[ri+1].ak!=-1) {
			copyv(&v,&route[ri+1].i);
			subv(&v,&route[ri].i);
			dest.x=v.y;
			dest.y=-v.x;
			dest.z=0;
			if (randK()>.5) mulv(&dest,-1);
			mulv(&dest,(2+randK()*3));
			addv(&dest,&route[ri].i);
			if (fabs(dest.x)<((WMAP-5)<<(NECHELLE-1)) && fabs(dest.y)<((WMAP-5)<<(NECHELLE-1))) {
				dest.z=zsolraz(dest.x,dest.y);
				prospectroute(&route[ri].i,&dest);
			}
		}
		affjauge(.75/(3.*150.));
	}
//	printf("%d road elemts for footpaths\n",routeidx-EndRoads);
/*	{
		vector u,v;
		u.x=-10*ECHELLE+2345; u.y=10*ECHELLE+1234; u.z=zsol(u.x,u.y);
		v.x=10*ECHELLE-2345; v.y=10*ECHELLE+1234; v.z=zsol(v.x,v.y);
		traceroute(&u,&v);
	}*/
	endinitroute();
//	printf("Nb Road elements : %d\n",routeidx);
	hashroute();
//	printf("Continuing world generation...\n");
	// des fermes et des usines
	for (i=0; i<(NBVILLAGES*10); i++) {
		vector pp;
		int ri;
		matrix m;
		do ri=EndRoads+randK()*(routeidx-EndRoads-1);
		while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
		copyv(&p,&route[ri].i);
		randomhm(&m);
		randomv(&pp);
		mulv(&pp,.5*ECHELLE);
		addv(&pp,&p);
		posem(&m,&pp);
		if (i&1) {
			pp.z=10;
			addnobjet(NBNAVIONS+NBBASES+2, &pp, &m, 1);
		} else {
			pp.z=20;
			addnobjet(NBNAVIONS+NBBASES+3, &pp, &m, 1);
		}
	}
	// des maisons au bord des routes
	for (i=0; i<200; i++) {
		vector pp;
		int ri;
		matrix m;
		do ri=EndRoads+randK()*(routeidx-EndRoads-1);
		while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
		copyv(&p,&route[ri].i);
		randomhm(&m);
		randomv(&pp);
		mulv(&pp,.3*ECHELLE);
		addv(&pp,&p);
		posem(&m,&pp);
		pp.z=48;
		addnobjet(NBNAVIONS+NBBASES+0, &pp, &m, 1);
	}
	// des moulins
	DebMoulins=nbobj;
	for (i=0; i<NBVILLAGES*2; i++) {
		vector pp;
		int ri;
		matrix m;
		do ri=EndRoads+randK()*(routeidx-EndRoads-1);
		while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
		copyv(&p,&route[ri].i);
		randomhm(&m);
		randomv(&pp);
		mulv(&pp,3*ECHELLE);
		addv(&pp,&p);
		pp.z=30;
		posem(&m,&pp);
		addnobjet(NBNAVIONS+NBBASES+4, &pp, &m, 1);
	}
	FinMoulins=nbobj;
	// des troupeaux de charolaises
	for (i=0; i<NBVILLAGES*2; i++) {
		int nbn=randK()*5+2;
		copyv(&p,&village[i%NBVILLAGES].p);
		for (k=0; k<2; k++) {
			vector pt;
			randomv(&pt);
			mulv(&pt,ECHELLE*6);
			addv(&pt,&p);
			pt.z=0;
			for (j=0; j<nbn; j++) {
				vector pp;
				matrix m;
				randomhm(&m);
				randomv(&pp);
				mulv(&pp,ECHELLE*.5*pow(nbn,.1));
				pp.z=12;
				addv(&pp,&pt);
				posem(&m,&pp);
				addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+5, &pp, &m, 1);
			}
		}
	}
	// des véhicules en décors
	voiture=(voiture_s*)malloc((NBVOITURES+1)*sizeof(voiture_s));
	for (i=0; i<NBVOITURES/4; i++) {
		voiture[i].r=randomvroute(&p);
		voiture[i].sens=1;
		p.z=15;
		voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+1, &p, &mat_id, 1);
		voiture[i].dist=-1;
		voiture[i].vit=5+15*randK();
	}
	for (; i<NBVOITURES/2; i++) {
		voiture[i].r=randomvroute(&p);
		voiture[i].sens=1;
		p.z=15;
		voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+4, &p, &mat_id, 1);
		voiture[i].dist=-1;
		voiture[i].vit=5+15*randK();
	}
	for (; i<NBVOITURES*8/10; i++) {
		voiture[i].r=randomvroute(&p);
		voiture[i].sens=1;
		p.z=15;
		voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+2, &p, &mat_id, 1);
		voiture[i].dist=-1;
		voiture[i].vit=10+10*randK();
	}
	for (; i<NBVOITURES; i++) {
		voiture[i].r=randomvroute(&p);
		voiture[i].sens=1;
		p.z=30;
		voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+3, &p, &mat_id, 1);
		voiture[i].dist=-1;
		voiture[i].vit=5+5*randK();
	}
	voiture[i].o=nbobj;
	// des tracteurs dans les champs
	for (i=0; i<50; i++) {
		matrix m;
		randomhm(&m);
		randomvferme(&p);
		posem(&m,&p);
		p.z=30;
		addnobjet(NBNAVIONS+NBBASES+NBMAISONS+3, &p, &m, 1);
	}
	// et des arbres
	for (i=0; i<150; i++) {
		matrix m;
		randomhm(&m);
		randomvferme(&p);
		posem(&m,&p);
		p.z=29;
		addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+3, &p, &m, 1);
	}
/*	for (i=0; i<1; i++) {
		matrix m;
		randomhm(&m);
		randomvferme(&p);
		p.z=20;
		addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+4, &p, &m, 1);
	}*/
	// des vionvions
	printf("NBBOT=%d\n",NBBOT);
	if ((bot=calloc(sizeof(*bot),NBBOT))==NULL) { perror("bot"); exit(-1); }
	printf("bmanu=%d\n",bmanu);
	bot[bmanu].camp=camp;
	bot[bmanu].navion=monvion-1;
	if (NetCamp()==-1) {printf("Net Error\n"); exit(-1); }
	IsFlying=SpaceInvaders;
	printf("Playing with %d planes & %d tanks\nPlayers :\n",NBBOT,NBTANKBOTS);
	for (i=0; i<NbHosts; i++) printf("%s, camp %d, in a %s\n",playbotname[i],bot[i].camp+1, viondesc[bot[i].navion].name);
	for (i=0; i<NBBOT; i++) {
		int c=i&3, b;
		if (i>=NbHosts) bot[i].camp=c;
		bot[i].babase=b=babaseo[0][(int)(randK()*3)][(int)bot[i].camp];
		if (!SpaceInvaders) {
			copyv(&p,&obj[b].pos);
			p.y+=i>=NbHosts?10:250;
			p.x+=300*(i>=NbHosts?(i>>2):i);
			p.z=20+zsol(p.x,p.y);
			copym(&m,&mat_id);
			m.x.x=0;
			m.x.y=-1;
			m.y.x=1;
			m.y.y=0;
		} else {
			randomhm(&m);
			copyv(&p,&vec_zero);
			if (bot[i].camp<2) p.y+=1000; else p.y-=1000;
			if (bot[i].camp&1) p.x+=1000; else p.x-=1000;
			if (i>=NbHosts) {
				p.x*=2*((i-NbHosts));
				p.y*=2*((i-NbHosts));
				p.z=0;
			} else {
				p.x+=(randK()-.5)*400;
				p.y+=(randK()-.5)*400;
				p.z=100*i;
			}
			p.z+=16000;
		}
		if (i>=NbHosts) bot[i].navion=randK()*NBNAVIONS;
		bot[i].vion=addnobjet(bot[i].navion,&p,&m, 0);
		bot[i].but.gear=!SpaceInvaders;
		bot[i].but.canon=0;
		bot[i].but.bomb=0;
		bot[i].but.gearup=0;
		bot[i].but.frein=0;
		bot[i].but.commerce=0;
		bot[i].anghel=0;
		bot[i].anggear=0;
	  	bot[i].xctl=bot[i].yctl=0;
		bot[i].thrust=SpaceInvaders?1:0;
		bot[i].manoeuvre=SpaceInvaders?4:0;
		bot[i].voltige=0; bot[i].gunned=-1;
		bot[i].fiulloss=bot[i].bloodloss=bot[i].motorloss=bot[i].aeroloss=0;
		bot[i].fiul=viondesc[bot[i].navion].fiulmax;
		bot[i].bullets=viondesc[bot[i].navion].bulletsmax;
		bot[i].cibv=bot[i].cibt=-1;
		bot[i].gold=i>NbHosts?30000:2000;
		armstate(i);
		if (!SpaceInvaders) copyv(&bot[i].vionvit,&vec_zero);
		else {
			copyv(&bot[i].vionvit,&obj[bot[i].vion].rot.x);
			mulv(&bot[i].vionvit,20);
			newnav(i);
		}
	}
	// des tanks
	for (i=0; i<NBTANKBOTS; i++) {
		if (randK()<.2) {
			randomvferme(&p);
			if (i&1) p.x*=.1; else p.y*=.1;
		} else {
			p.x+=(randK()-.5)*100;
			p.y+=(randK()-.5)*100;
		}
		p.z=zsol(p.x,p.y);
		if (p.y>0) vehic[i].camp=2; else vehic[i].camp=0;
		if (p.x>0) vehic[i].camp++;
		vehic[i].cibv=-1;
		copyv(&vehic[i].p,&p);
		p.z=20;
		vehic[i].o1=addnobjet(NBNAVIONS+NBBASES+NBMAISONS, &p, &mat_id, 1);
		vehic[i].o2=nbobj;
		vehic[i].moteur=0;
		vehic[i].cibt=-1;
		vehic[i].ang0=vehic[i].ang1=vehic[i].ang2=0;
		vehic[i].ocanon=0;
		vehic[i].nom=tankname;
	}
	// et des nuages
	for (i=0; i<70; i++) {
		int nbn=randK()*15+10;
		randomv(&p);
		mulv(&p,ECHELLE*(WMAP-5));
		p.z=randK()*5000+20000;
		for (j=0; j<nbn; j++) {
			vector pp;
			randomv(&pp);
			mulv(&pp,ECHELLE*pow(nbn,.3));
			pp.z/=2.;
			addv(&pp,&p);
			addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS, &pp, &mat_id, 0);
		}
	}
	// et de la fumée
	firstfumee=nbobj;
	for (i=0; i<NBMAXFUMEE; i++) {
		addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+1,&vec_zero,&mat_id,0);
	}
	rayonfumee=(uchar*)calloc(NBMAXFUMEE,sizeof(uchar));
	typefumee=(uchar*)calloc(NBMAXFUMEE,sizeof(uchar));
	printf("seed_end = %f\n",randK());
}
