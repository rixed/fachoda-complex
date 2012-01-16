#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <values.h>
#include <math.h>
#define NBMAXFACE 5000

facelight facesobj[NBMAXFACE*2];
vector ptsobj[NBMAXFACE*4];
int nbface=0,nbpt=0;
float size=0;
int centre,resize;

int addpoint(vector pt) { // scanne la liste de points et rajoute eventuelement ce point, retourne le numero
	int i,n;
	for (i=0; i<nbpt; i++)
		if (ptsobj[i].x==pt.x && ptsobj[i].y==pt.y && ptsobj[i].z==pt.z) break;
	if (i==nbpt) {
		ptsobj[nbpt].x=pt.x; ptsobj[nbpt].y=pt.y; ptsobj[nbpt].z=pt.z;
		nbpt++;
	}
	return(i);
}

void saveall(FILE *f) { // sauvegarde dans le fichier
	int i,j;
	float sizeactu;
	float xmin=MAXDOUBLE,xmax=-MAXDOUBLE,ymin=MAXDOUBLE,ymax=-MAXDOUBLE,zmin=MAXDOUBLE,zmax=-MAXDOUBLE;
	vector offset={0,0,0};
	printf("\nSauvegarde de %d FACES et %d POINTS\n", nbface, nbpt);
	// recentre
	for (i=0; i<nbpt; i++) {
		if (ptsobj[i].x>xmax) xmax=ptsobj[i].x;
		if (ptsobj[i].x<xmin) xmin=ptsobj[i].x;
		if (ptsobj[i].y>ymax) ymax=ptsobj[i].y;
		if (ptsobj[i].y<ymin) ymin=ptsobj[i].y;
		if (ptsobj[i].z>zmax) zmax=ptsobj[i].z;
		if (ptsobj[i].z<zmin) zmin=ptsobj[i].z;
	}
	printf("BOX = (%e/%e ; %e/%e ; %e;%e)\n",xmin,xmax,ymin,ymax,zmin,zmax);
	if (centre)
		for (i=0; i<nbpt; i++) {
			ptsobj[i].x -= (offset.x=(xmax+xmin)/2);
			ptsobj[i].y -= (offset.y=(ymax+ymin)/2);
			ptsobj[i].z -= (offset.z=(zmax+zmin)/2);
		}
	printf("offset : (%1.3e,%1.3e,%1.3e)\n",offset.x,offset.y,offset.z);
	if (xmax-xmin>ymax-ymin && xmax-xmin>zmax-zmin) sizeactu=xmax-xmin;
	else if (ymax-ymin>zmax-zmin) sizeactu=ymax-ymin;
	else sizeactu=zmax-zmin;
	printf("Taille initiale : %f\n",sizeactu);
	if (resize)
		for (i=0; i<nbpt; i++) {
			ptsobj[i].x *= size/sizeactu;
			ptsobj[i].y *= size/sizeactu;
			ptsobj[i].z *= size/sizeactu;
		}
	fwrite(&offset,sizeof(vector),1,f);
	fwrite(&nbpt,sizeof(int),1,f);
	fwrite(&nbface,sizeof(int),1,f);
	fwrite(&ptsobj,sizeof(vector),nbpt,f);
	fwrite(&facesobj,sizeof(facelight),nbface,f);
}
	
int main(int nbarg, char **arg) {
	FILE *in,*out=NULL;
	int i,j, np0, np1, np2, np3;
	float d;
	vector pt[4];
	char msg[260],objname[260];
	
	if (nbarg<2 || nbarg>4) { printf("dxfcompi fichin [taille_de_l'objet [flag_centre]]\n"); exit(-1); };
	if (resize=(nbarg>2)) printf("Resizing OK\n");
	centre=1; /*if (centre=(nbarg==4))*/ printf("Centering OK\n");
	if((in=fopen(arg[1],"r"))==NULL){ printf("erreur fichier in\n"); exit(-1); };
	if (resize) {
		sscanf(arg[2],"%lf",&size);
		if (size<=0) { printf("ga?\n"); exit(-1); };
	}
	while(1) {	
		while(1) {	// parse le header jusqu'à ce qu'on trouve une 3DFACE
			fscanf(in,"%d",&i);
			if (i==0) {
				fscanf(in,"%s",msg);
				if (strcmp(msg,"3DFACE")==0) break;
				if (strcmp(msg,"EOF")==0) { saveall(out); exit(0); };
			} else if (i==2) fscanf(in,"%s",msg);
		}
		while (1) {
			fscanf(in,"%d",&i);
			if (i==62) {
				fscanf(in,"%f",&d); //facesobj[nbface].color=facesobj[nbface+1].color=(int)d;	// la couleur
			} else if (i==8) {
				fscanf(in,"%s",msg); // le nom de l'objet
				if (strcmp(msg,objname)!=0) {	// le nom change !
					if (out!=NULL) {	// écrit le fichier
						saveall(out);
					//	facesobj[0].color=facesobj[1].color=facesobj[nbface].color;	// récupère la dernière couleure
						nbface=0; nbpt=0;
						fclose(out);
					};
					strcpy(objname,msg);
					printf("\nNouvel Objet : %s\n",objname);
					if ((out=fopen(objname,"w"))==NULL) {printf("Nique sa race!\n"); exit(-1); };
				}
			} else if (i>=10) break;
		}
		for (j=0; j<4; j++) {
			if (j) fscanf(in,"%d",&i);	// le premier à déjà été lu
			fscanf(in,"%f",&pt[j].x);
			fscanf(in,"%d",&i);
			fscanf(in,"%f",&pt[j].y);
			fscanf(in,"%d",&i);
			fscanf(in,"%f",&pt[j].z);
		}
		// rajouter ces deux faces
		np0=addpoint(pt[0]);
		np1=addpoint(pt[1]);
		np2=addpoint(pt[2]);
		np3=addpoint(pt[3]);
		facesobj[nbface].p[0]=np0;
		facesobj[nbface].p[1]=np1;
		facesobj[nbface].p[2]=np2;
		if (np0!=np1 && np0!=np2 && np1!=np2) nbface++;
		facesobj[nbface].p[0]=np2;
		facesobj[nbface].p[1]=np3;
		facesobj[nbface].p[2]=np0;
		if (np0!=np3 && np0!=np2 && np3!=np2) nbface++;
		printf("."); fflush(stdout);
	}
}
