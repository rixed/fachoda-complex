#ifndef TROISD_H
#define TROISD_H

#include <values.h>

typedef unsigned char uchar;
#include <stdlib.h>
#include <stdio.h>
#define min(a,b) ((a)<=(b)?(a):(b))
#define max(a,b) ((b)<=(a)?(a):(b))


#define NHASH 11	// 2048 eléments dans la table de hash
#define NBREPHASH	4	// nbr max d'éléments dans la meme case de la table
#define NBZEPS 20
#define NBVOITURES 400
#define BACKCOLOR 0xAC8DBD

#define NBKEYS 45 //56

#define NORMALDT 100000
#define NBMAXCLIENTS 100	// faire correspondre avec gate.c
#define DOGDISTMAX 9000
#define NBREPMAX 40
#define G_FACTOR .5
#define NTANKMARK 12	// 11 bits pour les No de tanks
#define vf 8	// 12
#define vfm (1<<vf)

#define MARGE 1	// au bord de la texture de map

#define NBPTSLISS 45
#define NBDECOUPLISS 8
#define NBNIVOLISS 4
#define cam obj[0]

#define NBNAVIONS 6
#define NBBASES 2
#define NBMAISONS 6
#define NBVEHICS 5
#define NBDECOS 7
#define NBZEPPELINS 1

#define NBMAXTIR 1200
#define NBMAXFUMEE 400
#define NBGRAVMAX 1000
#define NBFUMEESOURCE 40
#define NBPRIMES (4*5)
#define NBVILLAGES 10

#define NWMAP 7
#define WMAP (1<<NWMAP)
#define NMAP 5
#define SMAP (1<<NMAP)
#define NMAP2 3
#define SMAP2 (1<<NMAP2)
#define NECHELLE 12
#define ECHELLE (1<<NECHELLE)

//#define NBNOMVILLAGE 5

typedef enum { CAMERA, TIR, AVION, CIBGRAT, BOMB, PHARE, VEHIC, DECO, GRAV, NUAGE, FUMEE, TABBORD, ZEPPELIN } type_e;
typedef enum { PRESENT, BIPINTRO, SHOT, GEAR_DN, GEAR_UP, SCREETCH, MOTOR, HIT, MESSAGE, EXPLOZ, EXPLOZ2, TOLE, BIPBIP, BIPBIP2, BIPBIP3, FEU, TARATATA, ALLELUIA, ALERT, DEATH, PAIN, BRAVO } sample_e;

typedef struct {
	uchar b,g,r;
} pixel;
typedef struct {
	uchar b,g,r,u;
} pixel32;
#include "world.h"
typedef struct {
	char gear:1;
	char canon:1;
	char bomb:1;
	char flap:1;
	char gearup:1;
	char frein:1;
	char commerce:1;
	char repere:1;
} bouton_s;
typedef struct {
	int p[3];
	pixel color;
	vector norm;
} face;

typedef struct {	// utilisé dans les fichiers de data
	int p[3];	// les trois numéros de point dans la liste de point de l'objet
} facelight;

typedef struct {
	vector offset;	// centre de l'objet avant recentrage par dxfcompi (utile pour positioner les fils)
	int nbpts[NBNIVOLISS], nbfaces[NBNIVOLISS], pere, nobjet;
	type_e type;
	vector *(pts[NBNIVOLISS]), *(norm[NBNIVOLISS]);
	face *(fac[NBNIVOLISS]);
	float rayoncarac, rayoncollision, rayon;
	char fixe;
} modele;

typedef struct {
	short int model;		// No du modèle
	uchar type;
	vector pos;		// translation par rapport à l'obj de référence
	matrix rot;		// rotation par rapport à l'obj de reference
	short int objref;		// l'objet de référence pour la pos et la rot (-1=absolu)
	short int next,prec;	// lien sur l'objet suivant dans la liste du tri en Z
	short int ak;
	vector posc;
	float distance;	// eloignement en R2 a la camera
	vector t;	// position du centre dans le repere de la camera
	uchar aff:1;	// 0 = pas aff, 1=aff normal
} objet;
typedef struct {
	char *fn, *fnlight;
	int pere;	// relativement à la première pièce du modèle
	char plat:1;	// 1 si la piece est plate
	char bomb:3;	// 0 si pas bombe, 1 si light, 2 si HEAVY ! 3 = destructible à la bombe, pour instal terrestres
	char mobil;
	char platlight:1;
} piece_s;
typedef struct {
	int nbpieces;
	piece_s *piece;
	int firstpiece;
} nobjet_s;
typedef struct {
	int reward;
	char camp;
	short int no;
	int dt;
	char *endmsg;
} prime_s;
typedef struct {
	int o1, o2;
	char *nom;
	vector p;
} village_s;
typedef struct {
	short int navion,babase;	// Numéro de type d'avion, de base
	int vion;	// numéro de l'objet principal de l'avion
	char camp;	// 1 ou 2 ou -1 si détruit
//	char *nom;
	int nbomb;
	bouton_s but;
	vector vionvit;
	vector acc;
	float anghel, anggear;
	float vitlin;
	int bullets;
	float zs;
	float xctl,yctl,thrust;
	char manoeuvre;
	char voltige;
	vector u,v;
	matrix m;
	int cibt,cibv,a;
	float p,vc, df;
	uchar alterc;
	int fiul;
	int fiulloss;
	char motorloss;
	char aeroloss;
	int bloodloss;
	int gunned;
	float cap;
	int burning;
	int gold;
} bot_s;
typedef struct {
	short int o, cib[6];
	vector nav;
	float angz,angy,angx;
	float anghel;
	float vit;
} zep_s;
typedef struct {
	char camp;	// 1 ou 2 ou -1 si détruit.
	int o1, o2;
	char *nom;
	vector p;
	int moteur:1;
	int tir:1;
	int cibt,cibv;
	float ang0,ang1,ang2;
	int ocanon;
} vehic_s;

typedef struct {
	char *name;
	int nbpiecestete, prix,nbmoyeux, nbcharngearx,nbcharngeary,tabbord,firstcanon,nbcanon;
	uchar avant:1;
	uchar retract3roues:1;
	uchar oldtb:1;
	float motorpower,portf,port,derivk,profk,trainee;
	int bulletsmax, fiulmax;
	int roue[3];	// D,G,A
} viondesc_s;
typedef struct {
	short int o, b;
	vector vit;
} bombe_s;
typedef struct {
	int score;
	char name[30];
} HS_s;
typedef struct {
	short int o;
	vector vit;
	float a1,a2,ai1,ai2;
} debris_s;
typedef struct {
	int vague, camp, navion;
} alienpos_s;
typedef struct {
	char kc;
	char *name;
} kc_s;
typedef struct {
	int ak;
	vector i,i2;
	vect2dc e;
} route_s;
typedef struct {
	int r;
	short int o;
	char sens;
	float vit;
	int dist;
} voiture_s;
enum {VOICEGEAR, VOICESHOT, VOICEMOTOR, VOICEEXTER, VOICEALERT };
	
#include "proto.h"
#include "world.c"
#include "mapinline.c"

#endif
