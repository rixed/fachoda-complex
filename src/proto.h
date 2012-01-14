#include <X11/Xlib.h>
// naw.c
extern char hostname[250];
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
extern float AccelFactor;
extern char myname[30];
extern int fumeesource[], fumeesourceintens[];
extern debris_s debris[];
extern bombe_s *bombe;
extern int bombidx;
extern int babaseo[2][3][4];
extern int visu,visubomb,mapmode, accel, autopilot, bmanu, lapause, imgcount, visuobj;
extern double loinvisu, visuteta, visuphi;
extern uchar avancevisu,tournevisu,quitte,arme,AfficheHS;
extern matrix mat_id;
extern vector vac_diag, vec_zero, vec_g;
extern modele *mod;
extern objet *obj;
extern int nbobj, debtir;
extern double focale;
extern matrix Light;
extern char WINDOW,PHONG;
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
extern GC gc;
extern Display *disp;
extern Window win,root;
extern int bank, size, width, BufVidOffset, depth, XCONVERT;
extern pixel32 *videobuffer;
extern char *video;
extern void buffer2video(void);
extern XImage img;
extern char getscancode(void);
extern void initvideo(void);
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
extern void polyflat(vect2dlum *p1, vect2dlum *p2, vect2dlum *p3, int c);
extern void drawline(vect2dlum *p1, vect2dlum *p2, int col);
extern void drawline2(vect2d *p1, vect2d *p2, int col);
extern void calcposaind(int i);
extern void calcposa(void);
extern void renderer(int ak, int fast);
// txt.c
extern void pcharady(uchar m, int *v, int c, int off);
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
extern void pchar(uchar m, int x, int y, int c);
extern void pcharlent(uchar m, int x, int y, int c);
extern void pword(uchar *m, int x, int y, int c);
extern void pwordlent(uchar *m, int x, int y, int c);
extern void pstr(uchar *m, int y, int c);
extern void pstrlent(uchar *m, int y, int c);
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
// ran0.c
extern float randK(void);
extern long idum;
// map.c
extern void polyclip(vecic *p1, vecic *p2, vecic *p3);
extern void bougeflotte(void);
extern pixel zcol[256];
extern uchar *map;
extern uchar *mapmap;
extern short int *fomap;
extern pixel *colormap;
extern uchar *mapcol;
extern void initmap(void);
extern void draw_ground_and_objects(void);
extern void polygouro(vect2dc *p1, vect2dc *p2, vect2dc *p3);
extern float zsol(float,float);
extern float zsolraz(float,float);
// carte.c
extern vector repere[NBREPMAX];
extern int zoom, xcarte, ycarte, repidx;
extern void rendumap(void);
extern void rendumapbg(void);
extern int colcamp[4];
// robot.c
extern void newnav(int);
extern double cap(double x, double y);
extern bot_s *bot;
extern vehic_s *vehic;
extern zep_s *zep;
extern voiture_s *voiture;
extern void robotvehic(int v);
extern void armstate(int b);
extern void robot(int b);
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
extern int IsFlying;
extern char GUS;
extern float soundthrust;
extern void control(int b);
extern void controlvehic(int v);
extern void controlepos(int i);
extern void controlzep(int z);
// mapping.c
extern void polymap(vect2dm *p1, vect2dm *p2, vect2dm *p3);
extern void initmapping(void);
extern int *mapping;
extern uchar *preca;
extern void polyphong(vect2dlum *p1, vect2dlum *p2, vect2dlum *p3, int c);
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
extern pixel *SolMap;
extern double SolPicPh[20], SolPicOmega[20];
extern int *SolImgAdy;
extern int phix;
extern void initsol(void);
extern void affsoleil(vector *L);
// sound
extern char sound;
extern int opensound(void);
extern int loadsample(sample_e samp, char * fn, char loop);
extern void exitsound(void);
extern void playsound(int voice, sample_e samp, float freq, float vol, int pan);
extern void stopsound(int voice, sample_e samp, float vol);
// ravages.c
extern int collision(int p, int o);
extern int kelkan(int o);
extern void hitgun(int,int);
extern void explose(int oc, int i);
// net.c
extern void inittime(void);
extern void deltatime(void);
extern int NbHosts,MonoMode;
extern int NetInit(char *);
extern int NetRead(void);
extern void NetSend(void);
extern int NetCamp(void);
extern int NetClose(void);
extern long int DT, TotalDT;
// present.c
extern void affpresent(int,int);
extern char *scenar[4][4][2];
extern void redefinekeys(void);
extern int present(void);
extern void animpresent(void);
extern int invaders(void);
// extern int vague[4][4][4];	// vague, camp, colonne
extern int myc, myv, myt;
// code.as
extern void MMXPhongInit(int aa,int intcol);
extern void MMXFlatInit(void);
extern void MMXSaveFPU(void);
extern void MMXAddSat(int*,int);
extern void MMXAddSatC(int*,int);
extern void MMXSubSat(int*,int);
extern void MMXRestoreFPU(void);
extern void MMXFlat(int *dest, int nbr, int c);
extern void MMXFlatTransp(int *dest, int nbr, int c);
extern void MMXMemSetInt(int *deb, int coul, int n);
extern void MMXAddSatInt(int *deb, int coul, int n);
extern void MMXCopyToScreen(int *dest, int *src, int sx, int sy, int width);
extern void MMXCopy(int *dest, int *src, int nbr);
extern void MMXGouroPreca(int,int,int);
extern void MMXGouro(void);
extern void fuplot(int x, int y, int r);
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
