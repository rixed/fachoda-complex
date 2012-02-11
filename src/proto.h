// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
/* Copyright 2012
 * This file is part of Fachoda.
 *
 * Fachoda is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fachoda is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fachoda.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PROTO_H_120116
#define PROTO_H_120116

#include <string.h>
#include <stdbool.h>
#include "gtime.h"

#define MIN(a,b) ((a)<=(b)?(a):(b))
#define MAX(a,b) ((b)<=(a)?(a):(b))
#define ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

#define NHASH 11    // 2048 eléments dans la table de hash
#define NBREPHASH   4   // nbr max d'éléments dans la meme case de la table
#define NBZEPS 20
#define NBVOITURES 400
#define BACKCOLOR 0xAC8DBD

#define NBKEYS 45 //56

#define NBMAXCLIENTS 100    // faire correspondre avec gate.c
#define DOGDISTMAX 9000
#define NBREPMAX 40
#define ONE_METER 100.  // dist unit seams to be approx the cm
#define G (3. * ONE_METER)  // actual gravity is of course 10, but we like it smaller so that planes can fly slower
#define G_FACTOR .5
#define NTANKMARK 12    // 11 bits pour les No de tanks
#define vf 8    // 12
#define vfm (1<<vf)

#define MARGE 1 // au bord de la texture de map

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

// Total world map length
#define NWMAP 7
#define WMAP (1<<NWMAP)
// Length of 1 world tile
#define NECHELLE 12
#define ECHELLE (1<<NECHELLE)

//#define NBNOMVILLAGE 5

typedef enum { CAMERA, TIR, AVION, CIBGRAT, BOMB, PHARE, VEHIC, DECO, GRAV, NUAGE, FUMEE, TABBORD, ZEPPELIN } type_e;
typedef enum { PRESENT, BIPINTRO, SHOT, GEAR_DN, GEAR_UP, SCREETCH, LOW_SPEED, MOTOR, HIT, MESSAGE, EXPLOZ, EXPLOZ2, TOLE, BIPBIP, BIPBIP2, BIPBIP3, FEU, TARATATA, ALLELUIA, ALERT, DEATH, PAIN, BRAVO, NB_SAMPLES } sample_e;
enum voice { VOICEGEAR, VOICESHOT, VOICEMOTOR, VOICEEXTER, VOICEALERT, NB_VOICES };

typedef unsigned char uchar;

#define MAT_ID { {1, 0, 0},  {0, 1, 0},  {0, 0, 1} }
typedef struct {
    uchar b,g,r;
} pixel;
static inline int color_of_pixel(pixel c) {
    return (c.r<<16) + (c.g<<8) + (c.b);
}
typedef struct {
    uchar b,g,r,u;
} pixel32;
typedef struct {
    float x,y,z;
} vector;
#define PRIVECTOR "f,%f,%f"
#define PVECTOR(v) (v).x, (v).y, (v).z
typedef struct {
    int x,y,z;
} veci;
#define PRIVECI "f,%f,%f"
#define PVECI(v, p) ((float)v.x)/(1<<p), ((float)v.y)/(1<<p), ((float)v.z)/(1<<p)
typedef struct {
    veci v;
    pixel c;
} vecic;
typedef struct {
    int x, y;
} vect2d;
typedef struct {
    vect2d v;
    int xl, yl;
} vect2dlum;
typedef struct {
    vect2d v;
    pixel c;
} vect2dc;
typedef struct {
    vect2d v;
    uchar mx,my;
} vect2dm;
typedef struct {
    vector x,y,z;
} matrix;

typedef struct {
    char gear:1;    // how we want the gears (1 = down)
    char canon:1;
    char bomb:1;
    char flap:1;
    char gearup:1;  // if the gears are currently up
    char frein:1;
    char business:1;
    char repere:1;
} bouton_s;
typedef struct {
    int p[3];
    pixel color;
    vector norm;
} face;

typedef struct {    // utilisé dans les fichiers de data
    int p[3];   // les trois numéros de point dans la liste de point de l'objet
} facelight;

typedef struct {
    vector offset;  // centre de l'objet avant recentrage par dxfcompi (utile pour positioner les fils)
    int nbpts[NBNIVOLISS], nbfaces[NBNIVOLISS], pere, nobjet;
    type_e type;
    vector *(pts[NBNIVOLISS]), *(norm[NBNIVOLISS]);
    face *(fac[NBNIVOLISS]);
    float rayoncarac, rayoncollision, rayon;
    char fixe;
} modele;

typedef struct {
    uchar type; // FIXME: use mod[obj.modele].type?
    vector pos;     // translation par rapport à l'obj de référence
    matrix rot;     // rotation par rapport à l'obj de reference
    int next,prec;  // lien sur l'objet suivant dans la liste du tri en Z
    int objref;     // l'objet de référence pour la pos et la rot (-1=absolu)
    short int model;        // No du modèle
    short int ak;
    vector posc;
    float distance; // eloignement en R2 a la camera
    vector t;   // position du centre dans le repere de la camera
    uchar aff:1;    // 0 = pas aff, 1=aff normal
} objet;
typedef struct {
    char *fn, *fnlight;
    int pere;   // relativement à la première pièce du modèle
    char plat:1;    // 1 si la piece est plate
    char bomb:3;    // 0 si pas bombe, 1 si light, 2 si HEAVY ! 3 = destructible à la bombe, pour instal terrestres
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
    int dt;
    char *endmsg;
    int no;
    char camp;
} prime_s;
typedef struct {
    int o1, o2;
    char *nom;
    vector p;
} village_s;
typedef struct {
    short int navion,babase;    // Numéro de type d'avion, de base
    int vion;   // numéro de l'objet principal de l'avion
    char camp;  // 1 ou 2 ou -1 si détruit
    int nbomb;
    bouton_s but;
    vector vionvit;
    vector acc;
    float anghel, anggear;
    float vitlin;
    int bullets;
    gtime last_shot;
    float zs;
    float xctl,yctl,thrust;
    /* Normaly a bot is in maneuver (ie going to bomb something), but it can be
     * interrupted to fight/escape before returning to it's plan. */
    enum aerobatic {
        MANEUVER,   // see below field
        TURN,       // high rate turn
        RECOVER,    // level the wings
        CLIMB_VERT, // high rate climb
        TAIL,       // follow target's six
        CLIMB,      // regain altitude
    } aerobatic;
    enum maneuver {
        PARKING,
        TAXI,
        LINE_UP,
        TAKE_OFF,
        NAVIG,
        DIVE_N_BOMB,
        NOSE_UP,
        ILS_1,
        ILS_2,
        ILS_3,
        EVADE,
        HEDGEHOP,
        BOMBING,
    } maneuver;
    vector u,v; // navpoint pos and orientation (FIXME: rename!)
    matrix m;
    int cibt,cibv,a;
    float p;
    float target_speed;
    float target_alt;   // absolute altitude
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
    bool is_flying;
} bot_s;

char const *aerobatic_2_str(enum aerobatic);
char const *maneuver_2_str(enum maneuver);

typedef struct {
    int o, cib[6];
    gtime last_shot;
    vector nav;
    float angz,angy,angx;
    float anghel;
    float vit;
} zep_s;
typedef struct {
    char camp;  // 1 ou 2 ou -1 si détruit.
    int o1, o2;
    char *nom;
    vector p;
    int moteur:1;
    int tir:1;
    int cibt;   // object targeted as a ground target
    int cibv;   // plane target
    float ang0; // cap in the map
    float ang1, ang2;    // orientation of the turret
    int ocanon;
    gtime last_shot;
} vehic_s;

typedef struct {
    char *name;
    int nbpiecestete, prix,nbmoyeux, nbcharngearx,nbcharngeary,tabbord,firstcanon,nbcanon;
    uchar avant:1;
    uchar retract3roues:1;
    uchar oldtb:1;
    float motorpower, lift, drag;
    int bulletsmax, fiulmax;
    int roue[3];    // D,G,A
} viondesc_s;
typedef struct {
    int o, b;
    vector vit;
} bombe_s;
typedef struct {
    int score;
    char name[30];
} HS_s;
typedef struct {
    int o;
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
    int o;
    char sens;
    float vit;
    int dist;
} voiture_s;

// naw.c
extern int const NbHosts, MonoMode;
extern vector ExplozePos; extern int Exploze;
extern int DebMoulins, FinMoulins;
extern void akref(int ak,vector *r);
extern int akpos(vector *p);
extern void basculeY(int o, float a);
extern void basculeX(int o, float a);
extern void basculeZ(int o, float a);
extern char (*playbotname)[30];
extern int resurrect(void);
extern int NBBOT,NBTANKBOTS, camp, AllowResurrect, Easy, Gruge, ViewAll, SpaceInvaders, monvion, lang, Dark, Fleuve, MouseCtl, Accident, Smooth;
extern float CtlSensitiv, CtlSensActu, CtlAmortis, CtlYequ;
extern char myname[30];
extern int fumeesource[], fumeesourceintens[];
extern debris_s debris[];
extern bombe_s *bombe;
extern int bombidx;
extern int babaseo[2][3][4];
extern enum view_type {
    // internal views
    VIEW_IN_PLANE,
    VIEW_DOGFIGHT,
    // external views
    VIEW_STANDING,
    // cycling external views
    VIEW_ROTATING_PLANE,
    VIEW_BEHIND_PLANE,
    VIEW_PLANE_FROM_ABOVE,
    VIEW_ANYTHING_CHEAT,
    VIEW_ROTATING_BOMB,
    NB_VIEWS,
} view;
enum view_type next_external_view(enum view_type);
extern int visubomb,mapmode, accel, autopilot, bmanu, lapause, imgcount, visuobj;
extern double loinvisu, visuteta, visuphi;
extern uchar avancevisu,tournevisu,quitte,arme,AfficheHS;
extern matrix mat_id;
extern vector vac_diag, vec_zero, vec_g;
extern modele *mod;
extern objet *obj;
extern int nbobj, debtir;
extern double focale;
extern matrix Light;
extern char PHONG;
extern float TROPLOIN,TROPLOIN2;
extern int _DX,_DY,SX,SY,SYTB,SXTB,SIZECERCLE,POLYMAX,TBY;
extern int nbtir;
extern void addobjet(int, vector *, matrix *, int, uchar);
extern int visubot;
extern int gold;
extern int gunner[NBMAXTIR];
extern short int vieshot[NBMAXTIR];
extern uchar *rayonfumee;
extern uchar *typefumee;
extern int firstfumee;
extern void tournevion(int v, float d, float p, float g);
// video_interf
extern int bank, size, width, BufVidOffset, depth;
extern pixel32 *videobuffer;
extern char *video;
extern void buffer2video(void);
extern char getscancode(void);
extern void initvideo(bool fullscreen);
extern int kread(unsigned n);
extern int kreset(unsigned n);
extern void xproceed(void);
// renderer.c
extern void calcposrigide(int o);
extern void calcposarti(int o, matrix *m);
extern void drawlinetb(vect2d *p1, vect2d *p2, int col);
extern void initrender(void);
extern void plot(int x, int y, int r);
extern void mixplot(int x, int y, int r, int g, int b);
extern void plotmouse(int x,int y);
extern void plotcursor(int x,int y);
extern void cercle(int x, int y, int radius, int c);
extern bool polyflat(vect2d *p1, vect2d *p2, vect2d *p3, pixel color);
extern void drawline(vect2d const *restrict p1, vect2d const *restrict p2, int col);
extern void drawline2(vect2d *p1, vect2d *p2, int col);
extern void calcposaind(int i);
extern void calcposa(void);
enum render_part { GROUND, CLOUDS, SKY, ALL };  // ALL = GROUND+SKY
extern void renderer(int ak, enum render_part);
#define VEC_DEBUG
#ifdef VEC_DEBUG
enum debug_vector {
    DBG_VEC_SPEED, DBG_VEC_GRAVITY, DBG_VEC_THRUST, DBG_VEC_DRAG, DBG_VEC_LIFT,
    NB_DBG_VECS
};
extern vector debug_vector[NB_DBG_VECS][2]; // start, stop
extern void draw_debug(void);
#endif
// txt.c
extern void pcharady(int m, int *v, int c, int off);
extern int TextClipX1,TextClipX2,TextColfont;
extern void pnumchar(int n, int x, int y, int c);
extern void pnum(int n, int x, int y, int c, char just);
extern void pnuma(int n, int x, int y, int c, char just);
extern int SizeCharY;
extern int SizeBigCharY, SizeBigCharX, SizeBigChar;
extern void loadbigfont(char *fn);
extern void pbignumchar(int n, int x, int y, int c);
extern void pbignum(int n, int x, int y, char just, char tot, char dolard);
extern void loadfont(char *fn, int nx, int ny, int cy);
extern void pchar(int m, int x, int y, int c);
extern void pcharlent(int m, int x, int y, int c);
extern void pword(char const *m, int x, int y, int c);
extern void pwordlent(char const *m, int x, int y, int c);
extern void pstr(char const *m, int y, int c);
extern void pstrlent(char const *m, int y, int c);
// modele.c
extern viondesc_s viondesc[];
extern nobjet_s nobjet[];
extern void LoadModeles(void);
extern int addnobjet(int na, vector *p, matrix *m, uchar);
// radio.c
extern prime_s prime[];
extern village_s village[];
extern void clearprime(void);
extern void newprime(void);
extern char *nomvillage[];
extern char msgactu[1000];
extern int msgactutime;
extern int campactu;
// map.c
extern void polyclip(vecic *p1, vecic *p2, vecic *p3);
extern pixel *colormap;
extern uchar *mapcol;
// carte.c
extern vector repere[NBREPMAX];
extern int zoom, xcarte, ycarte, repidx;
extern void rendumap(void);
extern void rendumapbg(void);
extern int colcamp[4];

// FIXME: defined in robot.c but should go elsewhere
extern bot_s *bot;
extern vehic_s *vehic;
extern zep_s *zep;
extern voiture_s *voiture;

//tableaubord.c
extern int xsoute,ysoute,xthrust,ythrust,rthrust,xspeed,yspeed,rspeed,xalti,yalti,ralti,xinclin,yinclin,hinclin,dxinclin,xgear,ygear,rgear;
extern void rectangle(int *v, int rx, int ry, int c);
extern void disque(int *v, int r, int c);
extern void rectangletb(pixel32 *v, int rx, int ry, int c);
extern void disquetb(pixel32 *v, int r, int c);
extern void rectangleZ(int x, int y, int rx, int ry, int c);
extern void disqueZ(int x, int y, int r, int c);
extern void loadtbtile(char *fn);
extern void drawtbback(void);
extern void drawtbcadrans(int b);
extern int lx,ly,lz;
extern short int sxtbtile, sytbtile;
extern pixel32 *tbtile, *tbback, *tbback1, *tbback2;
extern uchar *tbz;
extern int *tbwidth;
// control
#define BEST_LIFT_SPEED (2.5 * ONE_METER)    // according to control.c
#define MIN_SPEED_FOR_LIFT 120.
extern float soundthrust;
extern void control_plane(int b, float dt_sec);
extern void control_vehic(int v, float dt_sec);
extern void controlepos(int i);
extern void control_zep(int z, float dt_sec);
// mapping.c
extern void polymap(vect2dm *p1, vect2dm *p2, vect2dm *p3);
extern void initmapping(void);
extern int *mapping;
extern void polyphong(vect2dlum *p1, vect2dlum *p2, vect2dlum *p3, pixel c);
// manuel.c
extern void NextDogBot(void);
extern void manuel(int b);
extern uchar but1released,but2released;
extern int xmouse,ymouse,bmouse;
extern int DogBot;
extern vector DogBotDir;
extern float DogBotDist;
// soleil
extern void animsoleil(void);
extern void initsol(void);
extern void affsoleil(vector *L);
// ravages.c
extern int collision(int p, int o);
extern int kelkan(int o);
// shot_idx: idx in objs list (substract debtir for index in gunner/vieshot)
extern bool hitgun(int obj_idx, int shot_idx);
extern void explose(int oc, int i);
// present.c
extern void affpresent(int,int);
extern char *scenar[4][4][2];
extern void redefinekeys(void);
extern int present(void);
extern void animpresent(void);
extern int invaders(void);
// extern int vague[4][4][4];   // vague, camp, colonne
extern int myc, myv, myt;
// code.as
extern void MMXAddSat(int*,int);
extern void MMXAddSatC(int *,int);
extern void MMXSubSat(int*,int);
extern void MMXFlatTransp(int *dest, int nbr, int c);
extern void MMXMemSetInt(int *deb, int coul, int n);
extern void MMXAddSatInt(int *deb, int coul, int n);
extern void MMXCopyToScreen(int *dest, int *src, int sx, int sy, int width);
extern void MMXCopy(int *dest, int *src, int nbr);
extern uchar *BigFont;
extern uchar font[112][10];
// keycodes
extern kc_s gkeys[NBKEYS];
// route
extern int largroute[3];
extern short (*map2route)[NBREPHASH];
extern void hashroute(void);
extern int NbElmLim, EndMotorways, EndRoads;
extern route_s *route;
extern int routeidx;
extern void initroute(void);
extern void endinitroute(void);
extern void prospectroute(vector *i,vector *f);
extern void traceroute(vector *i,vector *f);
// drawroute
extern void drawroute(int bb/*, vecic *ptref*/);
// init
extern void affjauge(float j);
extern void initworld(void);
extern void randomhm(matrix *m);

static inline int add_sat(int a, int b, int max)
{
    int const c = a + b;
    if (c > max) return max;
    else if (c < 0) return 0;
    return c;
}

static inline float proj1(float p, float z) {
    return (p * focale) / z;
}
static inline void proj(vect2d *e, vector *p) {
    e->x = _DX + proj1(p->x, p->z);
    e->y = _DY + proj1(p->y, p->z);
}
static inline void proji(vect2d *e, veci *p) {
    e->x=_DX+p->x*focale/p->z;
    e->y=_DY+p->y*focale/p->z;
}
static inline void addv(vector *r, vector const *a) { r->x+=a->x; r->y+=a->y; r->z+=a->z; }
static inline void addvi(veci *r, veci const *a) { r->x+=a->x; r->y+=a->y; r->z+=a->z; }
static inline void subv(vector *r, vector const *a) { r->x-=a->x; r->y-=a->y; r->z-=a->z; }
static inline void subvi(veci *r, veci const *a) { r->x-=a->x; r->y-=a->y; r->z-=a->z; }
static inline void negvi(veci *r) { r->x = -r->x; r->y = -r->y; r->z = -r->z; }
static inline void mulv(vector *r, float a) { r->x*=a; r->y*=a; r->z*=a; }
static inline void copyv(vector *r, vector const *a) { r->x=a->x; r->y=a->y; r->z=a->z; }
static inline void copym(matrix *r, matrix const *a) { memcpy(r,a,sizeof(matrix)); }
static inline void mulm(matrix *r, matrix const *a) {
    matrix b;
    copym(&b, r);
    r->x.x = b.x.x*a->x.x+b.y.x*a->x.y+b.z.x*a->x.z;
    r->y.x = b.x.x*a->y.x+b.y.x*a->y.y+b.z.x*a->y.z;
    r->z.x = b.x.x*a->z.x+b.y.x*a->z.y+b.z.x*a->z.z;
    r->x.y = b.x.y*a->x.x+b.y.y*a->x.y+b.z.y*a->x.z;
    r->y.y = b.x.y*a->y.x+b.y.y*a->y.y+b.z.y*a->y.z;
    r->z.y = b.x.y*a->z.x+b.y.y*a->z.y+b.z.y*a->z.z;
    r->x.z = b.x.z*a->x.x+b.y.z*a->x.y+b.z.z*a->x.z;
    r->y.z = b.x.z*a->y.x+b.y.z*a->y.y+b.z.z*a->y.z;
    r->z.z = b.x.z*a->z.x+b.y.z*a->z.y+b.z.z*a->z.z;
}
static inline void mulm3(matrix *r, matrix const *c, matrix const *a) {
    matrix b;
    b.x.x = c->x.x*a->x.x+c->y.x*a->x.y+c->z.x*a->x.z;
    b.y.x = c->x.x*a->y.x+c->y.x*a->y.y+c->z.x*a->y.z;
    b.z.x = c->x.x*a->z.x+c->y.x*a->z.y+c->z.x*a->z.z;
    b.x.y = c->x.y*a->x.x+c->y.y*a->x.y+c->z.y*a->x.z;
    b.y.y = c->x.y*a->y.x+c->y.y*a->y.y+c->z.y*a->y.z;
    b.z.y = c->x.y*a->z.x+c->y.y*a->z.y+c->z.y*a->z.z;
    b.x.z = c->x.z*a->x.x+c->y.z*a->x.y+c->z.z*a->x.z;
    b.y.z = c->x.z*a->y.x+c->y.z*a->y.y+c->z.z*a->y.z;
    b.z.z = c->x.z*a->z.x+c->y.z*a->z.y+c->z.z*a->z.z;
    copym(r, &b);
}
static inline void mulmt3(matrix *r, matrix const *c, matrix const *a) {    // c est transposée
    matrix b;
    b.x.x = c->x.x*a->x.x + c->x.y*a->x.y + c->x.z*a->x.z;
    b.y.x = c->x.x*a->y.x + c->x.y*a->y.y + c->x.z*a->y.z;
    b.z.x = c->x.x*a->z.x + c->x.y*a->z.y + c->x.z*a->z.z;
    b.x.y = c->y.x*a->x.x + c->y.y*a->x.y + c->y.z*a->x.z;
    b.y.y = c->y.x*a->y.x + c->y.y*a->y.y + c->y.z*a->y.z;
    b.z.y = c->y.x*a->z.x + c->y.y*a->z.y + c->y.z*a->z.z;
    b.x.z = c->z.x*a->x.x + c->z.y*a->x.y + c->z.z*a->x.z;
    b.y.z = c->z.x*a->y.x + c->z.y*a->y.y + c->z.z*a->y.z;
    b.z.z = c->z.x*a->z.x + c->z.y*a->z.y + c->z.z*a->z.z;
    copym(r, &b);
}

float norme(vector *u);
static inline float norme2(vector const *u){ return(u->x*u->x+u->y*u->y+u->z*u->z); }
static inline float scalaire(vector const *u, vector const *v){ return(u->x*v->x+u->y*v->y+u->z*v->z); }
static inline float renorme(vector *a) {
    float d = norme(a);
    if (d!=0) {a->x/=d; a->y/=d; a->z/=d; }
    return(d);
}
static inline void prodvect(vector const *a, vector const *b, vector *c) {
    c->x = a->y*b->z-a->z*b->y;
    c->y = a->z*b->x-a->x*b->z;
    c->z = a->x*b->y-a->y*b->x;
}
static inline void orthov(vector *a, vector *b) {
    float s=scalaire(a,b);
    a->x -= s*b->x;
    a->y -= s*b->y;
    a->z -= s*b->z;
}
static inline float orthov3(vector *a, vector *b, vector *r) {
    float s=scalaire(a,b);
    r->x = a->x-s*b->x;
    r->y = a->y-s*b->y;
    r->z = a->z-s*b->z;
    return(s);
}
static inline void mulmv(matrix *n, vector *v, vector *r) {
    vector t;
    copyv(&t,v);
    r->x = n->x.x*t.x+n->y.x*t.y+n->z.x*t.z;
    r->y = n->x.y*t.x+n->y.y*t.y+n->z.y*t.z;
    r->z = n->x.z*t.x+n->y.z*t.y+n->z.z*t.z;
}
static inline void mulmtv(matrix *n, vector *v, vector *r) {
    vector t;
    copyv(&t,v);
    r->x = n->x.x*t.x+n->x.y*t.y+n->x.z*t.z;
    r->y = n->y.x*t.x+n->y.y*t.y+n->y.z*t.z;
    r->z = n->z.x*t.x+n->z.y*t.y+n->z.z*t.z;
}
static inline void neg(vector *v) { v->x=-v->x; v->y=-v->y; v->z=-v->z; }
static inline void subv3(vector *a, vector *b, vector *restrict r) {    // il faut r!=a,b
    r->x = a->x-b->x;
    r->y = a->y-b->y;
    r->z = a->z-b->z;
}
static inline void addv3(vector *a, vector *b, vector *restrict r) {    // il faut r!=a,b
    r->x = a->x+b->x;
    r->y = a->y+b->y;
    r->z = a->z+b->z;
}
static inline void cap_dist(vector *a, float dist) {
#   define CAP(x) if ((x) > dist) x = dist; else if ((x) < -dist) x = -dist;
    CAP(a->x);
    CAP(a->y);
    CAP(a->z);
}
void randomv(vector *v);
static inline void randomm(matrix *m) {
    randomv(&m->x);
    renorme(&m->x);
    m->y.x=-m->x.y;
    m->y.y=+m->x.x;
    m->y.z=-m->x.z;
    orthov(&m->y,&m->x);
    renorme(&m->y);
    prodvect(&m->x,&m->y,&m->z);
}

#endif
