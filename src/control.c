#include <stdlib.h>
#include <math.h>
#include "3d.h"

int IsFlying;

float soundthrust=0;

void controlepos(int i) {
	int xk,yk,ak;
	xk=(int)floor(obj[i].pos.x/ECHELLE)+(WMAP>>1);
	yk=(int)floor(obj[i].pos.y/ECHELLE)+(WMAP>>1);
	if (xk<0) {obj[i].pos.x=-(WMAP<<(NECHELLE-1))+10; xk=0;}
	else if (xk>=WMAP) {obj[i].pos.x=(WMAP<<(NECHELLE-1))-10; xk=WMAP-1;}
	if (yk<10) {obj[i].pos.y=-((WMAP/2-10)<<NECHELLE)+10; yk=10;}
	else if (yk>=WMAP-10) {obj[i].pos.y=((WMAP/2-10)<<NECHELLE)-10; yk=WMAP-1-10;}
	if (mod[obj[i].model].fixe!=-1) {	// immobile ?
		ak=xk+(yk<<NWMAP);
		if (ak!=obj[i].ak) {
			if (obj[i].next!=-1) obj[obj[i].next].prec=obj[i].prec;
			if (obj[i].prec!=-1) obj[obj[i].prec].next=obj[i].next;
			else fomap[obj[i].ak]=obj[i].next;
			obj[i].next=fomap[ak];
			if (fomap[ak]!=-1) obj[fomap[ak]].prec=i;
			obj[i].prec=-1;
			fomap[ak]=i;
			obj[i].ak=ak;
		}
	}
}
void control(int b) {
	int i,j;
	int o1 = bot[b].vion;
	int o2 = o1+nobjet[bot[b].navion].nbpieces;
	vector v; matrix m;
	double k, rt, deriv=0, prof=0, gouv=0, zs;
	double vx,vy,vz;
	if (bot[b].camp==-1) return;
	bot[b].cap=cap(obj[o1].rot.x.x,obj[o1].rot.x.y);
	bot[b].vitlin=scalaire(&bot[b].vionvit,&obj[bot[b].vion].rot.x);
	bot[b].xctl+=bot[b].bloodloss*.02*(randK()-.5);
	if (b<NbHosts) bot[b].xctl+=bot[b].aeroloss*(b&1?.01:-.01);
	bot[b].yctl+=bot[b].bloodloss*.02*(randK()-.5);
	if (b<NbHosts) bot[b].yctl-=bot[b].aeroloss*.005;
	// controles
	if (bot[b].thrust<0) bot[b].thrust=0;
	if (bot[b].thrust>1) bot[b].thrust=1;
	if (bot[b].xctl<-1) bot[b].xctl=-1;
	if (bot[b].xctl>1) bot[b].xctl=1;
	if (bot[b].yctl<-1) bot[b].yctl=-1;
	if (bot[b].yctl>1) bot[b].yctl=1;
	if (viondesc[bot[b].navion].nbcharngearx==0 && viondesc[bot[b].navion].nbcharngeary==0) bot[b].but.gear=1;
	// fiul
	if ((bot[b].fiul-=(int)(10.*bot[b].thrust*AccelFactor)+bot[b].fiulloss*AccelFactor)<0) {
		bot[b].fiul=0;
		bot[b].thrust=0;
	}
	vy=scalaire(&bot[b].vionvit,&obj[bot[b].vion].rot.y);
	vz=scalaire(&bot[b].vionvit,&obj[bot[b].vion].rot.z);
	vx=scalaire(&bot[b].vionvit,&obj[bot[b].vion].rot.x);
	// motorisation
	k=bot[b].thrust*110*(1-bot[b].motorloss/128.);	// vitesse du flux d'air crée par les hélices (dépend de la puissance moteur 30=200km/h)
	if (k>bot[b].vitlin) {
		copyv(&v,&obj[bot[b].vion].rot.x);
		mulv(&v,/*AccelFactor**/(k-bot[b].vitlin)*viondesc[bot[b].navion].motorpower);	// dépend de l'aérodynamisme des appareils
		addv(&bot[b].vionvit,&v);
	}
	// poid
	copyv(&v,&vec_g);
	mulv(&v,G_FACTOR/**AccelFactor*/);
	addv(&bot[b].vionvit,&v);
	// trainées
	mulmtv(&obj[bot[b].vion].rot,&bot[b].vionvit,&v);
	k=viondesc[bot[b].navion].trainee+.00004*bot[b].nbomb;
	if (!bot[b].but.gearup) k+=.0001;
	if (bot[b].but.flap) k+=.00005;
	v.x*=1-k*fabs(v.x);//*AccelFactor;
	v.y*=1-.0045*fabs(v.y);//*AccelFactor;
	v.z*=1-.005*fabs(v.z);//*AccelFactor;
	mulmv(&obj[bot[b].vion].rot,&v,&bot[b].vionvit);
	// effet du flux d'air sur les ailes
	{	// des effets de moment angulaire
		// 1 lacet pour les ailes
		float kx;
		vector u;
		// les ailes aiment bien etre de front (girouette)
		prof=(vx>0?.04:-.04)*vz;
		/*if (b<NbHosts)*/ gouv=.04*vy;
		// rouli et profondeur
		if (vx<5) kx=0;
		else if (vx<10) kx=.001066666*(vx-5)*(vx-5)*(vx-5)*(vx-5);
		else if (vx<30) kx=1-(1/300.)*(vx-20)*(vx-20);
		else kx=-.003*vx+.75666;
		deriv+=(bot[b].xctl*viondesc[bot[b].navion].derivk*kx)/(1.+.5*bot[b].nbomb);
		deriv+=-.01*obj[bot[b].vion].rot.y.z;
		if (Easy || b>=NbHosts) kx*=1.2;
		prof+=bot[b].yctl*viondesc[bot[b].navion].profk*kx;
		tournevion(bot[b].vion, deriv*AccelFactor, prof*AccelFactor, gouv*AccelFactor);
		// miracle des airs, les ailes portent...
		if (bot[b].vitlin<12) kx=0;
		else if (bot[b].vitlin<20) kx=.002*vx*vx;
		else kx=.8997*exp(-.005875*vx);
		copyv(&u,&obj[bot[b].vion].rot.z);
		mulv(&u,/*AccelFactor**/((bot[b].but.flap?viondesc[bot[b].navion].portf:viondesc[bot[b].navion].port)*kx*(1-bot[b].aeroloss/128.)));
		addv(&bot[b].vionvit,&u);
	}
	// contact des roues
	zs=obj[bot[b].vion].pos.z-bot[b].zs;
	for (rt=0, j=0, i=0; i<3; i++) {	// ordre droite, gauche, arrière
		float zr=obj[bot[b].vion+viondesc[bot[b].navion].roue[i]].pos.z-zs+bot[b].vionvit.z*AccelFactor;
		if (zr<0) {
			int fum;
			j+=1<<i;
			if (bot[b].but.gearup) mulv(&bot[b].vionvit,.9);
			else {
				mulmtv(&obj[bot[b].vion].rot,&bot[b].vionvit,&v);
				v.x*=1-(bot[b].but.frein?.015:.0004)*fabs(v.x);
				v.y*=1-.5*fabs(v.y);
				v.z*=1-.05*fabs(v.z);
				mulmv(&obj[bot[b].vion].rot,&v,&bot[b].vionvit);
			}
			if (zr<rt) rt=zr;
			if (((Easy || b>=NbHosts) && (zr<-20*AccelFactor || (zr<-13*AccelFactor && bot[b].but.gearup) || (zr<-10*AccelFactor && mapmapm(obj[bot[b].vion].ak)!=0))) || (zr<-13*AccelFactor || (zr<-10*AccelFactor && bot[b].but.gearup) || (zr<-8*AccelFactor && mapmapm(obj[bot[b].vion].ak)!=0))) {
				explose(bot[b].vion,bot[b].vion);
				return;
			}
			if (zr<-2*AccelFactor) {
			//	printf("b=%d zr=%f af=%f\n",b,zr,AccelFactor);
				float t=randK()-.5;
				if (b==visubot) {
					if (!bot[b].but.gearup) playsound(VOICEGEAR,SCREETCH,1+t*.08,-.3*(zr+2*AccelFactor),0);
					else playsound(VOICEGEAR,TOLE,1+t*.08,-.3*(zr+2*AccelFactor),0);
				};
			}
			if (zr<-1*AccelFactor) {// || (bot[b].but.gearup && vx>3)) {
				for (fum=0; rayonfumee[fum] && fum<NBMAXFUMEE; fum++);
				if (fum<NBMAXFUMEE) {
					rayonfumee[fum]=1;
					typefumee[fum]=1;	// type poussière jaune
					copyv(&obj[firstfumee+fum].pos,&obj[bot[b].vion+viondesc[bot[b].navion].roue[i]].pos);
					controlepos(firstfumee+fum);
				}
			} else {
				// nice landing !
				if (b==bmanu && IsFlying && fabs(obj[bot[b].vion].rot.x.x)>.8) {
					subv3(&obj[bot[b].babase].pos,&obj[bot[b].vion].pos,&v);
					if (norme2(&v)<ECHELLE*ECHELLE*1.5) {
						bot[b].gold+=300;
						playsound(VOICEEXTER,BRAVO,1,1,0);
					}
				}
			}
			if (b==bmanu) IsFlying=0;
		}
	}
	if (j) {
		bot[b].vionvit.z-=rt/AccelFactor;
		if (j&3  && !(j&4)) {
			if (viondesc[bot[b].navion].avant) basculeY(bot[b].vion,rt*-.06);
			else basculeY(bot[b].vion,rt*.06);
		} else if (!(j&3) && j&4) {
			if (viondesc[bot[b].navion].avant) basculeY(bot[b].vion,rt*.06);
			else basculeY(bot[b].vion,rt*-.06);
		}
		if (j&1 && !(j&2)) basculeX(bot[b].vion,rt*-.06);
		else if (j&2 && !(j&1)) basculeX(bot[b].vion,rt*.06);
		if (j&3) basculeZ(bot[b].vion,bot[b].xctl*AccelFactor*-.05);	// petite gruge pour diriger en roulant
		if (j && bot[b].vitlin<.5) {	// on recharge ?
			copyv(&v,&obj[bot[b].babase].pos);
			subv(&v,&obj[bot[b].vion].pos);
			if (norme2(&v)<ECHELLE*ECHELLE*.1) {
				int prix,amo;
				amo=bot[b].gold;
				if (amo+bot[b].bullets>viondesc[bot[b].navion].bulletsmax) amo=viondesc[bot[b].navion].bulletsmax-bot[b].bullets;
				if (amo) {
					bot[b].gold-=amo;
					bot[b].bullets+=amo;
				}
				amo=bot[b].gold*1000;
				if (amo+bot[b].fiul>viondesc[bot[b].navion].fiulmax) amo=viondesc[bot[b].navion].fiulmax-bot[b].fiul;
				if (amo/1000) {
					bot[b].gold-=amo/1000;
					bot[b].fiul+=amo;
				}
				for (i=o1; i<o2; i++) {
					int mo=obj[i].model;
					if (obj[i].objref==bot[b].babase && mod[mo].type==BOMB) {
						prix=200;
						if (prix>bot[b].gold) break;
						bot[b].gold-=prix;
						obj[i].objref=bot[b].vion;
						obj[i].type=mod[obj[i].model].type;
						copym(&obj[i].rot,&mat_id);
						copyv(&obj[i].pos,&mod[mo].offset);
						bot[b].fiul=viondesc[bot[b].navion].fiulmax;
						armstate(b);
					}
				}
				if (bot[b].fiulloss) {
					bot[b].gold-=bot[b].fiulloss;
					bot[b].fiulloss=0;
				}
				if (bot[b].motorloss) {
					bot[b].gold-=bot[b].motorloss*10;
					bot[b].motorloss=0;
				}
				if (bot[b].aeroloss) {
					bot[b].gold-=bot[b].aeroloss*10;
					bot[b].aeroloss=0;
				}
				if (bot[b].bloodloss) {
					bot[b].gold-=bot[b].bloodloss;
					bot[b].bloodloss=0;
				}
				if (bot[b].burning) {
					bot[b].gold-=bot[b].burning*10;
					bot[b].burning=0;
				}
			//	if (b==bmanu && bot[b].gold<0) { printf("MORT AUX PAUVRES!\n"); exit(-1); }
				if (bot[b].but.commerce) {
					i=bot[b].vion;
					do {
						int v;
						bot[b].gold+=viondesc[bot[b].navion].prix;
						do {
							i=obj[i].next;
							if (i==-1) i=fomap[obj[bot[b].vion].ak];
						} while (obj[i].type!=AVION || mod[obj[i].model].fixe!=0);
						bot[b].navion=mod[obj[i].model].nobjet;
						bot[b].gold-=viondesc[bot[b].navion].prix;
						for (j=v=0; v<NBBOT; v++) {
							if (v!=b && bot[v].vion==i) { j=1; break; }
						}
					} while (j || bot[b].gold<0);
					if (i!=bot[b].vion) {
						bot[b].vion=i;
						armstate(b);
						playsound(VOICEEXTER,TARATATA,1+(bot[b].navion-1)*.1,1,0);
					}
				}
			}
		}
	} else /* !j */ if (b==bmanu && bot[b].zs>500) IsFlying=1;
	// avance vionvion
	copyv(&v,&bot[b].vionvit);
	mulv(&v,AccelFactor);
	addv(&obj[bot[b].vion].pos,&v);
	controlepos(bot[b].vion);
	// ET LE RESTE SUIT...
	// Moyeux d'hélice
	for (i=1; i<viondesc[bot[b].navion].nbmoyeux+1; i++) {
		m.x.x=1; m.x.y=0; m.x.z=0;
		m.y.x=0; m.y.y=cos(bot[b].anghel); m.y.z=sin(bot[b].anghel);
		m.z.x=0; m.z.y=-sin(bot[b].anghel); m.z.z=cos(bot[b].anghel);
		calcposarti(bot[b].vion+i,&m);
	}
	bot[b].anghel+=bot[b].thrust*AccelFactor;
	if (b==visubot && bot[b].thrust!=soundthrust) {
		soundthrust=bot[b].thrust;
		playsound(VOICEMOTOR,MOTOR,pow(soundthrust,.02),1,0);
	}
	// charnières des essieux
	if (bot[b].but.gear) {
		if (bot[b].but.gearup) {
			bot[b].but.gearup=0;
			if (b==visubot) playsound(VOICEGEAR,GEAR_DN,1,1,0);
			for (j=0; j<(viondesc[bot[b].navion].retract3roues?3:2); j++) obj[bot[b].vion+viondesc[bot[b].navion].roue[j]].aff=1;
		}
		bot[b].anggear-=.1*AccelFactor;
		if (bot[b].anggear<0) bot[b].anggear=0;
	} else if (!bot[b].but.gearup) {
		if (b==visubot && bot[b].anggear<.1) playsound(VOICEGEAR,GEAR_UP,1,1,0);
		bot[b].anggear+=.1*AccelFactor;
		if (bot[b].anggear>1.5) {
			bot[b].anggear=1.5; bot[b].but.gearup=1;
			for (j=0; j<(viondesc[bot[b].navion].retract3roues?3:2); j++) obj[bot[b].vion+viondesc[bot[b].navion].roue[j]].aff=0;
		}
	}
	for (j=i; j<viondesc[bot[b].navion].nbcharngearx+i; j++) {
		m.y.y=cos(bot[b].anggear);
		m.y.z=((j-i)&1?-1:1)*sin(bot[b].anggear);
		m.z.y=((j-i)&1?1:-1)*sin(bot[b].anggear);
		m.z.z=cos(bot[b].anggear);
		calcposarti(bot[b].vion+j,&m);
	}
	m.y.x=0; m.y.y=1; m.y.z=0;
	m.z.y=0;
	for (i=j; i<j+viondesc[bot[b].navion].nbcharngeary; i++) {
		m.x.x=cos(bot[b].anggear); m.x.z=((i-j)&1?1:-1)*sin(bot[b].anggear);
		m.z.x=((i-j)&1?-1:1)*sin(bot[b].anggear); m.z.z=cos(bot[b].anggear);
		calcposarti(bot[b].vion+i,&m);
	}
	// charnière de profondeur
	m.x.x=cos(bot[b].yctl);
	m.x.z=-sin(bot[b].yctl);
	m.z.x=sin(bot[b].yctl);
	m.z.z=cos(bot[b].yctl);
	calcposarti(bot[b].vion+i,&m);
	// charnières de rouli
	m.x.x=cos(bot[b].xctl);
	m.x.z=-sin(bot[b].xctl);
	m.z.x=sin(bot[b].xctl);
	m.z.z=cos(bot[b].xctl);
	calcposarti(bot[b].vion+i+1,&m);
	m.x.z=-m.x.z;
	m.z.x=-m.z.x;
	calcposarti(bot[b].vion+i+2,&m);
	for (i+=3; i<nobjet[mod[obj[bot[b].vion].model].nobjet].nbpieces; i++) {
		if (obj[bot[b].vion+i].objref!=-1) calcposrigide(bot[b].vion+i);
	}
	// tirs ?
	if (bot[b].but.canon && nbtir<NBMAXTIR && bot[b].bullets>0) {
		if (++bot[b].alterc>=4) bot[b].alterc=0;
		if (bot[b].alterc<viondesc[bot[b].navion].nbcanon) {
			copyv(&v,&obj[bot[b].vion].rot.x);
			mulv(&v,44);
			addv(&v,&obj[bot[b].vion+viondesc[bot[b].navion].firstcanon+bot[b].alterc].pos);
			if (b==visubot) playsound(VOICESHOT,SHOT,1+(randK()-.5)*.08,1,bot[b].alterc&1?-128:127);
			else randK();
			gunner[nbobj-debtir]=b;
			vieshot[nbobj-debtir]=80;
			addobjet(0, &v, &obj[bot[b].vion].rot, -1, 0);
			nbtir++;
			bot[b].bullets--;
		}
	}
	if (bot[b].but.bomb) {
		for (i=bot[b].vion; i<bot[b].vion+nobjet[bot[b].navion].nbpieces && (obj[i].type!=BOMB || obj[i].objref!=bot[b].vion); i++);
		if (i<bot[b].vion+nobjet[bot[b].navion].nbpieces) {
			if (b==visubot) playsound(VOICEGEAR,BIPBIP2,1.1,.8,0);
			obj[i].objref=-1;
			for (j=0; j<bombidx && bombe[j].o!=-1; j++);
			if (j>=bombidx) bombidx=j+1;
			copyv(&bombe[j].vit,&bot[b].vionvit);
			bombe[j].o=i;
			bombe[j].b=b;
			bot[b].nbomb--;
		}
	}
	if (bot[b].but.repere && bot[b].camp==bot[bmanu].camp) {
		copyv(&repere[repidx],&obj[bot[b].vion].pos);
		if (++repidx>NBREPMAX) repidx=0;
	}
	bot[b].but.bomb=bot[b].but.canon=bot[b].but.commerce=bot[b].but.repere=0;
}
void controlvehic(int v) {
	vector p,u;
	int o=vehic[v].o1, i;
	matrix m;
	double c,s;
	if (vehic[v].camp==-1) return;
	c=cos(vehic[v].ang0);
	s=sin(vehic[v].ang0);
	m.x.x=c; m.x.y=s; m.x.z=0;
	m.y.x=-s; m.y.y=c; m.y.z=0;
	m.z.x=0; m.z.y=0; m.z.z=1;
	copym(&obj[o].rot,&m);
	if (vehic[v].moteur) {
		copyv(&p,&obj[o].rot.x);
		mulv(&p,5*AccelFactor);
		addv(&obj[o].pos,&p);
		obj[o].pos.z=zsol(obj[o].pos.x,obj[o].pos.y);
		controlepos(o);
	}
	c=cos(vehic[v].ang1);
	s=sin(vehic[v].ang1);
	m.x.x=c; m.x.y=s; m.x.z=0;
	m.y.x=-s; m.y.y=c; m.y.z=0;
	calcposarti(o+1,&m);
	c=cos(vehic[v].ang2);
	s=sin(vehic[v].ang2);
	m.x.x=c; m.x.y=0; m.x.z=s;
	m.y.x=0; m.y.y=1; m.y.z=0;
	m.z.x=-s;m.z.y=0; m.z.z=c;
	calcposarti(o+2,&m);
	for (i=o+3; i<vehic[v].o2; i++) calcposrigide(i);
	if (vehic[v].tir && nbtir<NBMAXTIR) {
		copyv(&p,&obj[o+3+vehic[v].ocanon].rot.x);
		mulv(&p,90);
		copyv(&u,&obj[o+3+vehic[v].ocanon].pos);
		addv(&p,&u);
		gunner[nbobj-debtir]=v|(1<<NTANKMARK);
		vieshot[nbobj-debtir]=70;
		addobjet(0, &p, &obj[o+3+vehic[v].ocanon].rot, -1, 0);
		nbtir++;
	}
}

void tiradonf(int z, vector *c, int i) {
	vector p;
	matrix m;
	float s;
	if (nbtir<NBMAXTIR) {
		copyv(&p,c);
		renorme(&p);
		s=scalaire(&p,&obj[zep[z].o].rot.y);
		if (i<3) s=-s;
		if (s<.5) return;
		copyv(&m.x,&p);
		m.y.x=m.y.y=m.y.z=1;
		orthov(&m.y,&m.x);
		renorme(&m.y);
		prodvect(&m.x,&m.y,&m.z);
		mulv(&p,40);
		addv(&p,&obj[zep[z].o+5+i].pos);
		gunner[nbobj-debtir]=-1;	// passe inapercu?
		vieshot[nbobj-debtir]=90;
		addobjet(0, &p, &m, -1, 0);
		nbtir++;
	}
}

void controlzep(int z) {	// fait office de routine robot sur les commandes aussi
	int i;
	vector v;
	matrix m;
	float dir;
	float cx,cy,cz,sx,sy,sz, zs, angcap=0, angprof=0;
	static float phazx=0, phazy=0, phazz=0, balot=0;
	balot+=.1/NBZEPS;
	if (obj[zep[z].o].pos.z>50000) return;
	zs=zsol(obj[zep[z].o].pos.x,obj[zep[z].o].pos.y);
	if (zep[z].vit>1) {
		// dégonfle
		zep[z].angy+=.4*sin(phazy+=randK())*AccelFactor;
		zep[z].angx+=.4*sin(phazx+=randK())*AccelFactor;
		zep[z].angz+=.4*sin(phazz+=randK())*AccelFactor;
		obj[zep[z].o].pos.z+=20;
	} else {
		// commandes
		copyv(&v,&zep[z].nav);
		subv(&v,&obj[zep[z].o].pos);
		if (norme2(&v)<2000) {
			zep[z].nav.x=(WMAP<<NECHELLE)*randK()*(SpaceInvaders?.1:.7);
			zep[z].nav.y=(WMAP<<NECHELLE)*randK()*(SpaceInvaders?.1:.7);
			zep[z].nav.z=3000+randK()*30000+zsolraz(v.x,v.y);
		} else {
			dir=cap(v.x,v.y)-zep[z].angz;
			if (dir<-M_PI) dir+=2*M_PI;
			else if (dir>M_PI) dir-=2*M_PI;
			angcap=-dir*.9;
			angprof=10*(v.z-obj[zep[z].o].pos.z)/(1.+max(fabs(v.y),fabs(v.x)));
			if (obj[zep[z].o].pos.z-zs<3000) angprof=1;
			if (angprof>1) angprof=1;
			else if (angprof<-1) angprof=-1;
			zep[z].vit=1;
		}
		// déplacement
		zep[z].angy-=angprof*.005*zep[z].vit*AccelFactor;
		if (zep[z].angy>.4) zep[z].angy=.4;
		else if (zep[z].angy<-.4) zep[z].angy=-.4;
		zep[z].angz-=angcap*.015*zep[z].vit*AccelFactor;
		zep[z].angx=.2*sin(balot)*AccelFactor;
	}
	cx=cos(zep[z].angx);
	sx=sin(zep[z].angx);
	cy=cos(zep[z].angy);
	sy=sin(zep[z].angy);
	cz=cos(zep[z].angz);
	sz=sin(zep[z].angz);
	obj[zep[z].o].rot.x.x=cy*cz;
	obj[zep[z].o].rot.x.y=sz*cy;
	obj[zep[z].o].rot.x.z=-sy;
	obj[zep[z].o].rot.y.x=cz*sx*sy-sz*cx;
	obj[zep[z].o].rot.y.y=sz*sx*sy+cz*cx;
	obj[zep[z].o].rot.y.z=sx*cy;
	obj[zep[z].o].rot.z.x=cz*cx*sy+sz*sx;
	obj[zep[z].o].rot.z.y=sz*cx*sy-cz*sx;
	obj[zep[z].o].rot.z.z=cx*cy;
	copyv(&v,&obj[zep[z].o].rot.x);
	mulv(&v,zep[z].vit*8*AccelFactor);
	addv(&obj[zep[z].o].pos,&v);
	if (obj[zep[z].o].pos.z<zs) obj[zep[z].o].pos.z=zs;
	controlepos(zep[z].o);
	// Hélices
	m.x.x=1; m.x.y=m.x.z=m.y.x=m.z.x=0;
	m.z.z=m.y.y=cos(zep[z].anghel);
	m.y.z=sin(zep[z].anghel);
	m.z.y=-m.y.z;
	calcposarti(zep[z].o+1,&m);
	m.y.z=-m.y.z; m.z.y=-m.z.y;
	calcposarti(zep[z].o+2,&m);
	zep[z].anghel+=.2*zep[z].vit*AccelFactor;
	// Charnières
	cz=cos(angcap);
	sz=sin(angcap);
	cy=cos(angprof);
	sy=sin(angprof);
	m.x.x=cz; m.x.y=sz; m.x.z=0;
	m.y.x=-sz; m.y.y=cz; m.y.z=0;
	m.z.x=0; m.z.y=0; m.z.z=1;
	calcposarti(zep[z].o+3,&m);
	m.x.x=cy; m.x.y=0; m.x.z=-sy;
	m.y.x=0; m.y.y=1; m.y.z=0;
	m.z.x=sy; m.z.y=0; m.z.z=cy;
	calcposarti(zep[z].o+4,&m);
	// cibles des canons
	for (i=0; i<6; i++) {
		if (zep[z].cib[i]==-1) {
			int nc=NBBOT*randK();
			copyv(&v,&obj[bot[nc].vion].pos);
			subv(&v,&obj[zep[z].o].pos);
			if (norme2(&v)<ECHELLE*ECHELLE*0.5) zep[z].cib[i]=nc;
		}
	}
	// Tirs
	for (i=5; i<5+6; i++) {
		if (zep[z].cib[i-5]!=-1) {
			subv3(&obj[bot[zep[z].cib[i-5]].vion].pos,&obj[zep[z].o+i].pos,&v);
			if (norme2(&v)>ECHELLE*ECHELLE*0.5) zep[z].cib[i-5]=-1;
			else if ((i+imgcount)&1) tiradonf(z,&v,i-5);		//dans gunner, laisser -1 ?
		}
	}
	// Le reste
	for (i=5; i<nobjet[mod[obj[zep[z].o].model].nobjet].nbpieces; i++) calcposrigide(zep[z].o+i);
}

