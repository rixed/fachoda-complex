#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "proto.h"
#include "map.h"

int collision(int p, int o){	// l'obj p est-il rentré dans l'obj o ?
	vector u;
	if (obj[o].type==NUAGE || obj[o].type==FUMEE || p==o || obj[p].ak!=obj[o].ak) return 0;
	subv3(&obj[p].pos,&obj[o].pos,&u);
	return norme(&u)<=mod[obj[o].model].rayoncollision+mod[obj[p].model].rayoncollision;
}
int kelkan(int o) {
	int c;
	if (obj[o].pos.y>0) c=2; else c=0;
	if (obj[o].pos.x>0) c++;
	return c;
}

void explose(int oc, int i) {
	int o1,o2=0,j,v=0,jk;
	int cmoi=NBBOT;
	vector p, vit;
	float np;
	for (j=0; j<NBBOT; j++) if ((bot[j].vion<=i && bot[j].vion+nobjet[bot[j].navion].nbpieces>i) || (i>=debtir && gunner[i-debtir]==j)) { cmoi=j; break; }
	copyv(&p,&obj[oc].pos);
	subv(&p,&obj[0].pos);
	np=renorme(&p);
	playsound(VOICEEXTER,EXPLOZ,1+(drand48()-.5)*.1,1./(1+np*np*1e-6),128*scalaire(&p,&obj[0].rot.x));
	copyv(&vit,&vec_zero);
	switch (obj[oc].type) {
	case CIBGRAT:
		o1=oc;
		while (mod[obj[o1].model].pere!=obj[o1].model) o1+=mod[obj[o1].model].pere-obj[o1].model;
		o2=o1+nobjet[mod[obj[o1].model].nobjet].nbpieces;
		if (cmoi<NBBOT && kelkan(o1)!=bot[cmoi].camp) bot[cmoi].gold+=900*drand48()*(o1==oc?3:1);	// les églises valent plus cher !
		break;
	case AVION:
		o1=oc;
		while (obj[o1].objref!=-1) o1=obj[o1].objref;
		o2=o1+nobjet[mod[obj[o1].model].nobjet].nbpieces;
		for (v=0; v<NBBOT; v++) if (bot[v].vion==o1) { copyv(&vit,&bot[v].vionvit); break; }
		if (cmoi<NBBOT) {
			if (v<NBBOT && bot[v].camp!=bot[cmoi].camp) bot[cmoi].gold+=1000;
		}
		if (v==visubot) {
			playsound(VOICEMOTOR,FEU,1,1,0);
			playsound(VOICEGEAR,DEATH,1+(drand48()-.5)*.15,1,0);
		} else drand48();
		break;
	case VEHIC:
		o1=oc;
		while (obj[o1].objref!=-1) o1=obj[o1].objref;
		o2=o1+nobjet[mod[obj[o1].model].nobjet].nbpieces;
		for (v=0; v<NBTANKBOTS; v++) if (vehic[v].o1==o1) break;
		if (cmoi<NBBOT) {
			if (v<NBTANKBOTS && vehic[v].camp!=bot[cmoi].camp) bot[cmoi].gold+=900;
		}
		break;
	default:
		o1=0;
		break;
	}
	if (o1) {
		switch (obj[o1].type) {
		case AVION:
			if (v<NBBOT) {
			//	printf("bot#%d, camp %d, destroyed\n",v,bot[v].camp);
				bot[v].camp=-1;
				if (i==oc && bot[v].gunned>=0 && bot[v].gunned<NBBOT) { cmoi=bot[v].gunned; bot[cmoi].gold+=1500; }
			}
			break;
		case VEHIC:
			if (v<NBTANKBOTS) {
			//	printf("vehic%d, camp %d, destroyed\n",v,vehic[v].camp);
				vehic[v].camp=-1;
				break;
			}
			break;
		}
//		printf("Obj %d sauvagely burst obj#%d out (type %d)\n",i,oc,obj[oc].type);
		copyv(&ExplozePos,&obj[i].pos);
		Exploze=1;
		if (obj[o1].pos.z<z_ground(obj[o1].pos.x,obj[o1].pos.y)+50) {
			obj[o1].model=nobjet[NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+2].firstpiece;	// CRATERE
			obj[o1].type=DECO;
			obj[o1].pos.z=5+z_ground(obj[o1].pos.x,obj[o1].pos.y);
			obj[o1].objref=-1;
			copym(&obj[o1].rot,&mat_id);
			jk=o1+1;
		} else jk=o1;
		for (j=jk; j<o2; j++) {
	//		obj[j].model=nobjet[NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+6].firstpiece;
		//	copyv(&obj[j].pos,&obj[o1].pos);
			obj[j].objref=-1;
			obj[j].type=DECO;
		}
		for (i=0, j=jk; i<NBGRAVMAX && j<o2; i++) {
			if (debris[i].o==-1) {
				debris[i].o=j;
				randomv(&debris[i].vit);
				if (jk==o1) mulv(&debris[i].vit,90);
				else {
					debris[i].vit.x*=20;
					debris[i].vit.y*=20;
					debris[i].vit.z*=80;
				}
				addv(&debris[i].vit,&vit);
				debris[i].a1=drand48()*M_PI*2;
				debris[i].a2=drand48()*M_PI*2;
				debris[i].ai1=debris[i].a1*.1;
				debris[i].ai2=debris[i].a2*.1;
				if (debris[i].vit.z>0) debris[i].vit.z=-debris[i].vit.z;
				j++;
			}
		}
		for (j=0; j<NBFUMEESOURCE && fumeesourceintens[j]; j++);
		if (j<NBFUMEESOURCE) {
			fumeesource[j]=o1;
			fumeesourceintens[j]=drand48()*2000;
		}
		for (j=0; j<NBPRIMES; j++) {
			if (prime[j].no>=o1 && prime[j].no<o2 && prime[j].reward>0) {
				if (cmoi<NBBOT) {
					bot[cmoi].gold+=prime[j].reward;
				}
				prime[j].reward=0;
			}
		}	
	}
}
void hitgun(int oc, int i) {
	int o1,j,tarif;
	int cmoi=gunner[i-debtir];
	switch (obj[oc].type) {
	case CIBGRAT:
		o1=oc;
		while (mod[obj[o1].model].pere!=obj[o1].model) o1+=mod[obj[o1].model].pere-obj[o1].model;
		break;
	case ZEPPELIN:
		if (cmoi==-1) return;
	case AVION:
	case VEHIC:
		o1=oc;
		while (obj[o1].objref!=-1) o1=obj[o1].objref;	// MARCHEA PLUS ?
		break;
	default:
		o1=0;
		break;
	}
	if (o1) {
		switch (obj[o1].type) {
		case CIBGRAT:
			if (cmoi<NBBOT && kelkan(o1)!=bot[cmoi].camp) if (drand48()<.05) bot[cmoi].gold+=60;
			if (o1!=oc && drand48()<.01) explose(o1,i);
			break;
		case AVION:
			for (j=0; j<NBBOT; j++) if (bot[j].vion==o1) {
				tarif=-(bot[j].fiulloss/4+bot[j].motorloss*8+bot[j].aeroloss*8+bot[j].bloodloss*2);
				if (j==bmanu) accel=0;
				if (j==visubot) playsound(VOICEGEAR,HIT,1+(drand48()-.5)*.1,drand48(),(drand48()-.5)*127); else {drand48(); drand48(); drand48();}
			//	printf("bot %d hit\n",j);
				if (drand48()<.1) bot[j].fiulloss+=drand48()*100;
				if (drand48()<.04) if ((bot[j].motorloss+=drand48()*10)<0) bot[j].motorloss=127;
				if (drand48()<.05) if ((bot[j].aeroloss+=drand48()*10)<0) bot[j].aeroloss=127;
				if (drand48()<.04) {
					bot[j].bloodloss+=drand48()*100;
					if (j==visubot) playsound(VOICEGEAR,PAIN,1+(drand48()-.2)*.1,1,0); else drand48();
				}
				if (drand48()<bot[j].nbomb/1000. || drand48()<.05) {
					bot[j].burning+=drand48()*1000;
					if (bot[j].burning>900) explose(o1,i);
					bot[j].fiulloss+=drand48()*200;
					if ((bot[j].motorloss+=drand48()*100)<0) bot[j].motorloss=127;
					if ((bot[j].aeroloss+=drand48()*100)<0) bot[j].aeroloss=127;
				}
				bot[j].gunned=gunner[i-debtir];
				tarif+=bot[j].fiulloss/4+bot[j].motorloss*8+bot[j].aeroloss*8+bot[j].bloodloss*2;
				if (cmoi<NBBOT) {
				  if (bot[j].camp!=bot[cmoi].camp) bot[cmoi].gold+=tarif;
				  else bot[cmoi].gold-=tarif;
				}
			}
			break;
		case VEHIC:
		/*	for (j=0; j<NBTANKBOTS; j++) if (vehic[j].o1==o1) {
				printf("vehic %d hit\n",j);
			}*/
			if (drand48()<.07) explose(o1,i);
			break;
		case ZEPPELIN:
			if (drand48()<.004) {
				for (i=0; i<NBZEPS && zep[i].o!=o1; i++);
				if (i<NBZEPS) {
					for (j=0; j<NBFUMEESOURCE && fumeesourceintens[j]; j++);
					if (j<NBFUMEESOURCE) {
						fumeesource[j]=zep[i].o;
						fumeesourceintens[j]=3000;
					}
					zep[i].vit=8;
				}
			}
		}
	}
}
