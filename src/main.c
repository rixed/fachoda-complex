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
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <values.h>
#include <assert.h>
#include <stdbool.h>
#include "heightfield.h"
#include "sound.h"
#include "gtime.h"
#include "keycodesdef.h"
#include "robot.h"
#include "file.h"

//#define PRINT_DEBUG

void memset32(int *deb, int coul, int n) {
    while (n--) *deb++ = coul;
}

struct matrix mat_id={{1,0,0},{0,1,0},{0,0,1}};
struct vector vac_diag={1,1,1}, vec_zero={0,0,0}, vec_g={0,0,-1};

int NBBOT=30;
int NBTANKBOTS=150;
int NBZEP=20;
struct model *mod;
struct object obj[50000];   // All nbobj objects
int nbobj;
int debtir, firstfumee, fumeedispo, DebMoulins, FinMoulins;
float AngleMoulin=0;
uchar *rayonfumee;
uchar *typefumee;
int fumeesource[MAX_SMOKE_SOURCES], fumeesourceintens[MAX_SMOKE_SOURCES];
double focale;
int camp=1, AllowResurrect=1, Easy=0, Gruge=0, ViewAll=0, SpaceInvaders=0, monvion=1, lang=1, Dark=-1, Fleuve=1, MouseCtl=1, Accident=1500, Smooth=7;
float CtlSensitiv=0.08, CtlSensActu=0, CtlAmortis=.9, CtlYequ=.1;
char myname[30];
struct matrix LightSol={ {-.7,0,-.7},{0,1,0},{.7,0,-.7}};
struct matrix Light;
struct vector explosion_pos;
bool explosion;
//
int babaseo[2][3][4];   // o1, o2 // base 1, base 2, base 3 // camp A,B,C,D
int gunner[MAX_SHOTS];
short int vieshot[MAX_SHOTS];
struct bomb *bomb;
int bombidx;
char (*playbotname)[30];
struct debris debris[MAX_DEBRIS];
// options changeables à la ligne de com :
int _DX,_DY,SX=400,SY=250,SYTB,SXTB,SIZECERCLE,TBY;
int nbtir;

// add an object to obj[]
void object_add(int mo, struct vector *p, struct matrix *m, int or, uchar sol)
{
    int xk,yk,ak;
    assert(nbobj < (int)ARRAY_LEN(obj));
    obj[nbobj].model=mo;
    obj[nbobj].type=mod[mo].type;
    obj[nbobj].aff=1;
    copyv(&obj[nbobj].pos,p);
    if (sol) obj[nbobj].pos.z+=z_ground(p->x,p->y, true);
    copym(&obj[nbobj].rot,m);
    obj[nbobj].objref=or;
    xk=(int)floor(p->x/TILE_LEN)+(MAP_LEN>>1);
    yk=(int)floor(p->y/TILE_LEN)+(MAP_LEN>>1);
    ak=xk+(yk<<LOG_MAP_LEN);
    obj[nbobj].ak=ak;
    obj[nbobj].next=map[ak].first_obj;
    map[ak].first_obj=nbobj;
    obj[nbobj].prec=-1;
    if (obj[nbobj].next!=-1) obj[obj[nbobj].next].prec=nbobj;
    nbobj++;
}

static void background_line(int *v,int sx,int dz,int z,int coul) {
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
            cz=-cz; //?
            v[x]=(r<<16)+(r<<8)+(cz&0xFF);
            z+=dz;
        } else for (x=0; x<sx; x++) {
            uchar r;
            cz=z>>8;    // z>>8 entre 0 et 64
            r=0x20+64-cz;
            cz=0x40+64-cz;
            v[x]=(r<<16)+(r<<8)+(cz&0xFF);
            z+=dz;
        }
    } else {
        memset32(v,coul,sx);
    }
}

static void background(void)
{
    int z1,z2, dz1,dz2;
    int dz, x, i, xfin;
    int zfront[3] = {64<<8,0,-64<<8};
#   define sol 0x1A6834
#   define soldark (0x1A6834>>1)
#   define ciel 0xA0A0C0
#   define cieldark 0x202040
    int coulfront[2][4] = {
        {ciel,0,sol,-1},
        {cieldark,0,soldark,-1}
    };
    int *vid;

    // Compute position of the horizon (notice it's artificially lowered by 30 pixels)
    z1=z2=(focale*obj[0].rot.z.z-_DY*obj[0].rot.y.z+30)*256;
    z1-=(_DX*obj[0].rot.x.z)*256;
    z2+=(_DX*obj[0].rot.x.z)*256;
    dz1=obj[0].rot.y.z*256;
    dz2=obj[0].rot.y.z*256;
    dz=(z2-z1)/SX;
#   define ZFINSOL ((-SX/4)<<8)
    for (vid=(int*)videobuffer; vid<(int*)videobuffer+SX*SY; vid+=SX, z1+=dz1, z2+=dz2) {
        if (z1>z2) {
            for (i=0, x=0; i<3 && x<SX; i++) {
                if (z1>zfront[i]) {
                    if (z2>zfront[i]) {
                        xfin=SX;
                        background_line(vid+x,xfin-x,dz,z1,coulfront[Dark][i]);
                        x=xfin;
                    } else {
                        if (z1-z2!=0) {
                            xfin=((z1-zfront[i])*SX)/(z1-z2);
                            if (xfin>SX) xfin=SX;
                        } else xfin=SX;
                        background_line(vid+x,xfin-x,dz,z1,coulfront[Dark][i]);
                        x=xfin;
                    }
                }
            }
            if (x<SX) background_line(vid+x,SX-x,dz,z1,coulfront[Dark][i]);
        } else {
            for (i=2, x=0; i>=0 && x<SX; i--) {
                if (z1<zfront[i]) {
                    if (z2<zfront[i]) {
                        xfin=SX;
                        background_line(vid+x,xfin-x,dz,z1,coulfront[Dark][i+1]);
                        x=xfin;
                    } else {
                        if (z1-z2!=0) {
                            xfin=((z1-zfront[i])*SX)/(z1-z2);
                            if (xfin>SX) xfin=SX;
                        } else xfin=SX;
                        background_line(vid+x,xfin-x,dz,z1,coulfront[Dark][i+1]);
                        x=xfin;
                    }
                }
            }
            if (x<SX) background_line(vid+x,SX-x,dz,z1,coulfront[Dark][i+1]);
        }
    }
}

int viewed_bot = 0, viewed_obj = 0;
enum view_type view = VIEW_IN_PLANE;
int viewed_bomb=0, map_mode=0, accelerated_mode=0, autopilot=0, game_paused=0, controled_bot, frame_count=0;
double extcam_dist = 2. * ONE_METER;    // external camera distance (ie. "zoom")
double sight_teta=0, sight_phi=0;   // direction of vision while in plane view
bool view_instruments, view_predef, prompt_quit, quit_game, draw_high_scores;
int selected_weapon = 0;


static void setup_camera(float dt_sec)
{
    static float view_angle = 0.;   // for the rotating view
    view_angle += 0.03;
    float n = 0.;
    int i;

    if (view == VIEW_ROTATING_BOMB) {
        if (!viewed_bomb || obj[viewed_bomb].objref!=-1) {
            for (i=bot[viewed_bot].vion; i<bot[viewed_bot].vion+n_object[bot[viewed_bot].navion].nbpieces && (obj[i].objref!=-1 || obj[i].type!=TYPE_BOMB); i++);
            if (i<bot[viewed_bot].vion+n_object[bot[viewed_bot].navion].nbpieces) {
                viewed_bomb = i;
            } else {
                viewed_bomb = 0;
                view = VIEW_IN_PLANE;
            }
        }
    }
    if (view == VIEW_ANYTHING_CHEAT && !Gruge) view = next_external_view(view);
    if (view == VIEW_DOGFIGHT) {
        if (viewed_bot != controled_bot) view = VIEW_IN_PLANE;
        else {
            if (DogBot==controled_bot || bot[DogBot].camp==-1) next_dog_bot();
            if (DogBot!=controled_bot && bot[DogBot].camp!=-1) {
                copyv(&DogBotDir,&obj[bot[DogBot].vion].pos);
                subv(&DogBotDir,&obj[bot[controled_bot].vion].pos);
                DogBotDist=renorme(&DogBotDir);
                if (DogBotDist>DOGDISTMAX) next_dog_bot();
                if (DogBotDist>DOGDISTMAX) view = VIEW_IN_PLANE;
            } else view = VIEW_IN_PLANE;
        }
    }
    if (view == VIEW_IN_PLANE || view == VIEW_DOGFIGHT) {   // afficher ou effacer la tete et la tab de bord
        for (i=0; i<plane_desc[bot[viewed_bot].navion].nbpiecestete; i++)
            obj[bot[viewed_bot].vion+n_object[bot[viewed_bot].navion].nbpieces-2-i].aff=0;
        obj[bot[viewed_bot].vion+plane_desc[bot[viewed_bot].navion].tabbord].aff=1;
    } else {
        for (i=0; i<plane_desc[bot[viewed_bot].navion].nbpiecestete; i++)
            obj[bot[viewed_bot].vion+n_object[bot[viewed_bot].navion].nbpieces-2-i].aff=1;
        obj[bot[viewed_bot].vion+plane_desc[bot[viewed_bot].navion].tabbord].aff=0;
    }
    switch (view) {
    case NB_VIEWS:
        assert(!"Invalid view");
    case VIEW_DOGFIGHT:
    case VIEW_IN_PLANE:
        // Compute static position of the head, relative to cockpit
        obj[0].pos = vec_zero;
        struct matrix ct;
        // Even in dogfight view we want to be able to focus on pannel or look toward predefined directions
        if (view == VIEW_IN_PLANE || view_instruments || view_predef) {
            ct.x = obj[bot[viewed_bot].vion].rot.y;
            neg(&ct.x);
            ct.y = obj[bot[viewed_bot].vion].rot.z;
            neg(&ct.y);
            ct.z = obj[bot[viewed_bot].vion].rot.x;
        } else {
            ct.z = DogBotDir;
            ct.y = obj[bot[viewed_bot].vion].rot.z;
            neg(&ct.y);
            orthov(&ct.y, &ct.z);
            renorme(&ct.y);
            prodvect(&ct.y, &ct.z, &ct.x);
        }
        if (view_instruments) {
            // Go for the instrument pannel
            struct vector v = ct.z;
            mulv(&v, 2.1);
            addv(&obj[0].pos, &v);
            v = ct.y;
            mulv(&v, 5.2);
            addv(&obj[0].pos, &v);
        } else {
            // Look in any direction (sight_teta/phi)
            struct matrix m;
            double ctt = cos(sight_teta);
            double st = sin(sight_teta);
            double cf = cos(sight_phi);
            double sf = sin(sight_phi);
            m.x.x = cf; m.y.x = sf*st;  m.z.x = -sf*ctt;
            m.x.y = 0;  m.y.y = ctt;    m.z.y = st;
            m.x.z = sf; m.y.z = -st*cf; m.z.z = cf*ctt;
            mulm(&ct, &m);
        }

        addv(&obj[0].rot.x, &ct.x);
        addv(&obj[0].rot.y, &ct.y);
        renorme(&obj[0].rot.x);
        orthov(&obj[0].rot.y, &obj[0].rot.x);
        renorme(&obj[0].rot.y);
        prodvect(&obj[0].rot.x, &obj[0].rot.y, &obj[0].rot.z);

        /* Now alter this static position to take into account acceleration.
         * Unfortunately, we do not know objs acceleration (nor velocity in the
         * general case), so we have to figure it out. */
        if (! accelerated_mode) {
            static struct vector prev_vit;
            struct vector acc;
            subv3(&bot[viewed_bot].vionvit, &prev_vit, &acc);
            mulv(&acc, .02/dt_sec);
            cap_dist(&acc, 3.);
            subv(&obj[0].pos, &acc);
            prev_vit = bot[viewed_bot].vionvit;
        }

        /* Smooth this position with the previous one */
        static struct vector prev_cam_pos = { .0, .0, .0 };
        struct vector diff;
        subv3(&obj[0].pos, &prev_cam_pos, &diff);
        mulv(&diff, .2);
        addv3(&prev_cam_pos, &diff, &obj[0].pos);
        prev_cam_pos = obj[0].pos;

        /* Finally, add to this position the actual position of the cockpit */
        addv(&obj[0].pos, &obj[bot[viewed_bot].vion+n_object[bot[viewed_bot].navion].nbpieces-1].pos);

        break;
    case VIEW_ROTATING_PLANE:
        obj[0].rot.y.x=0; obj[0].rot.y.y=0; obj[0].rot.y.z=-1;
        obj[0].rot.x.x=cos(view_angle); obj[0].rot.x.y=sin(view_angle); obj[0].rot.x.z=0;
        obj[0].rot.z.x=-sin(view_angle); obj[0].rot.z.y=cos(view_angle); obj[0].rot.z.z=0;
        copyv(&obj[0].pos,&obj[0].rot.z);
        mulv(&obj[0].pos,-extcam_dist);
        addv(&obj[0].pos,&obj[bot[viewed_bot].vion].pos);
        if (obj[0].pos.z<(n=30+z_ground(obj[0].pos.x,obj[0].pos.y, true))) obj[0].pos.z=n;
        break;
    case VIEW_PLANE_FROM_ABOVE:
        copym(&obj[0].rot,&mat_id);
        neg(&obj[0].rot.z); neg(&obj[0].rot.y);
        copyv(&obj[0].pos,&obj[bot[viewed_bot].vion].pos);
        obj[0].pos.z+=extcam_dist;
        break;
    case VIEW_ROTATING_BOMB:
        obj[0].rot.y.x=0; obj[0].rot.y.y=0; obj[0].rot.y.z=-1;
        obj[0].rot.x.x=cos(view_angle); obj[0].rot.x.y=sin(view_angle); obj[0].rot.x.z=0;
        obj[0].rot.z.x=-sin(view_angle); obj[0].rot.z.y=cos(view_angle); obj[0].rot.z.z=0;
        copyv(&obj[0].pos,&obj[0].rot.z);
        mulv(&obj[0].pos,-extcam_dist);
        addv(&obj[0].pos,&obj[viewed_bomb].pos);
        break;
    case VIEW_ANYTHING_CHEAT:
        obj[0].rot.y.x=0; obj[0].rot.y.y=0; obj[0].rot.y.z=-1;
        obj[0].rot.x.x=cos(view_angle); obj[0].rot.x.y=sin(view_angle); obj[0].rot.x.z=0;
        obj[0].rot.z.x=-sin(view_angle); obj[0].rot.z.y=cos(view_angle); obj[0].rot.z.z=0;
        copyv(&obj[0].pos,&obj[0].rot.z);
        mulv(&obj[0].pos,-extcam_dist);
        addv(&obj[0].pos,&obj[viewed_obj].pos);
        break;
    case VIEW_BEHIND_PLANE:
        obj[0].pos = obj[bot[viewed_bot].vion].pos;
        obj[0].rot.x = obj[bot[viewed_bot].vion].rot.y;
        neg(&obj[0].rot.x);
        obj[0].rot.y = obj[bot[viewed_bot].vion].rot.z;
        neg(&obj[0].rot.y);
        obj[0].rot.z = obj[bot[viewed_bot].vion].rot.x;
        struct vector p = obj[0].rot.z;
        mulv(&p, -(extcam_dist-80));
        addv(&obj[0].pos, &p);
        p = bot[viewed_bot].vionvit;
        mulv(&p, -1.);
        addv(&obj[0].pos, &p);
        if (obj[0].pos.z < (n = 30 + z_ground(obj[0].pos.x, obj[0].pos.y, true))) {
            obj[0].pos.z = n;
        }
        break;
    case VIEW_STANDING:
        subv3(&obj[bot[viewed_bot].vion].pos,&obj[0].pos,&obj[0].rot.z);
        renorme(&obj[0].rot.z);
        obj[0].rot.y.x=obj[0].rot.y.y=0;
        obj[0].rot.y.z=-1;
        orthov(&obj[0].rot.y,&obj[0].rot.z);
        renorme(&obj[0].rot.y);
        prodvect(&obj[0].rot.y,&obj[0].rot.z,&obj[0].rot.x);
        break;
    }
}

int main(int narg, char **arg)
{
    int i,j, dtradio=0;
    struct matrix m;
    int caisse=0, dtcaisse=0, oldgold=0, caissetot=0, maxgold=0, initradio=0;
    char *userid;
    FILE *file;
    struct high_score highscore[] = {
        { 4000, "George Brush"},
        { 3500, "Donald Ducksfield"},
        { 4000, "Ricardo Sandbag"},
        { 3500, "Geoffrey Maller"},
        { 3000, "Janis Karpinsku"},
        { 2500, "Carolyn Woed"},
        { 2000, "Steven Lou Jordan"},
        { 1000, "Charles Craner"},
        {  900, "Lynndie Andleng"},
        {  800, "Ivan Frederock"},
        {  700, "Jeremy Civil"},
        {  600, "Armin Cruise"},
        {  500, "Sabrina Hardman"},
    };
    unsigned maxrank = ARRAY_LEN(highscore);
    printf("Fachoda Complex - (C) 2000-2012 Cedric Cellier\n"
"This program comes with ABSOLUTELY NO WARRANTY.\n"
"This is free software, and you are welcome to redistribute it\n"
"under certain conditions; See http://www.gnu.org/licenses/gpl-3.0.html for details.\n"
"\n"
"   fullscreen      : Plain in fullscreen mode\n"
"   x n             : X size of the window (default : 320)\n"
"   y n             : Y size (default : 200)\n"
"   night           : Play at night\n"
"   camp 1|2|3|4    : the camp you want to fly for (default : 1)\n"
"   drone n         : number of drones (default : 30)\n"
"   tank n          : total number of tanks (default : 200)\n"
"   mortal          : forbids resurections (default : come back in another plane)\n"
"   name            : your name in the game (default : your user id)\n"
"   easy            : easy mode (default : guess)\n"
"   viewall         : view all enemies on the map (default : no)\n"
"   nosound         : turn sound OFF (default : sound on)\n"
"   killemAll       : Kill em All! (default : just be kool, this is a game)\n"
"   plane n         : The plane you start with : 1 for Dewoitine, 2 for Corsair, etc (default : 2)\n"
"   french          : Pour que les textes soient en francais (defaut : frenglish)\n"
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
    bool with_sound = true;
    bool fullscreen = false;

    for (i=1; i<narg; i++) {
        int c=0;
        while (arg[i][c]=='-' || arg[i][c]==' ') c++;
        if (0 == strcasecmp(&arg[i][c], "fullscreen")) fullscreen = true;
        else if (0 == strcasecmp(&arg[i][c],"night")) Dark=1;
        else if (0 == strcasecmp(&arg[i][c],"x")) {
            if (++i==narg || sscanf(arg[i],"%d",&SX)!=1) goto parse_error;
        } else if (0 == strcasecmp(&arg[i][c],"y")) {
            if (++i==narg || sscanf(arg[i],"%d",&SY)!=1) goto parse_error;
        } else if (0 == strcasecmp(&arg[i][c],"camp")) {
            if (++i==narg || sscanf(arg[i],"%d",&camp)!=1 || camp<1 || camp>4) goto parse_error;
        } else if (0 == strcasecmp(&arg[i][c],"drone")) {
            if (++i==narg || sscanf(arg[i],"%d",&NBBOT)!=1 || NBBOT<0 || NBBOT>100) goto parse_error;
        } else if (0 == strcasecmp(&arg[i][c],"tank")) {
            if (++i==narg || sscanf(arg[i],"%d",&NBTANKBOTS)!=1 || NBTANKBOTS<1 || NBTANKBOTS>500) goto parse_error;
        } else if (0 == strcasecmp(&arg[i][c],"mortal")) AllowResurrect=0;
        else if (0 == strcasecmp(&arg[i][c],"name")) {
            if (++i==narg) goto parse_error; else {
                for (j=0; j<(int)strlen(arg[i]) && j<29; j++) myname[j]=arg[i][j];
                myname[j]='\0';
                }
        } else if (0 == strcasecmp(&arg[i][c],"easy")) Easy=1;
        else if (0 == strcasecmp(&arg[i][c],"viewall")) ViewAll=1;
        else if (0 == strcasecmp(&arg[i][c],"nosound")) with_sound=false;
        else if (0 == strcasecmp(&arg[i][c],"nomouse")) MouseCtl=0;
        else if (0 == strcasecmp(&arg[i][c],"killemall")) SpaceInvaders=1;
        else if (0 == strcasecmp(&arg[i][c],"plane")) {
            if (++i==narg || sscanf(arg[i],"%d",&monvion)!=1 || monvion<1 || monvion>NB_PLANES) goto parse_error;
        } else if (0 == strcasecmp(&arg[i][c],"french")) lang=0;
        else if (0 == strcasecmp(&arg[i][c],"gruge")) Gruge=1;
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
//  TROPLOIN2=TROPLOIN*TROPLOIN;
    SX&=0xFFFFFFF8; SY&=0xFFFFFFFE;
    _DX=SX>>1; _DY=SY>>1;
//  SIZECERCLE=MIN(SX,SY)/10;
    SYTB=90;//SY/4;
    TBY=SY-SYTB;
    SXTB=SYTB*2; //SX>>1;
    focale=_DX;
    // Read saved highscores (from home)
    if ((file = file_open_try(".fachoda-highscores", getenv("HOME"), "r")) != NULL) {
        fread(&highscore, sizeof(struct high_score), ARRAY_LEN(highscore), file);
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
    for (i=0; i<NB_MARKS; i++) mark[i].x=MAXFLOAT;
    if (sound_init(with_sound)==-1) printf("Ce sera le monde du silence...\n");

    /*
        VIDEO
               */
    videobuffer = malloc(SX*SY*sizeof(*videobuffer));
    BufVidOffset = SX*sizeof(struct pixel32);
    initvideo(fullscreen);
    //  printf("Img bpp=%d\n",depth);
    drawtbback();
/*  fontname=XListFonts(disp,"-freefont-cooper-*-*-*-*-*-*-100-*-*-*-*-*",1,&i);
    xfont=XLoadQueryFont(disp,fontname[0]);
    XFreeFontNames(fontname);
    XSetState(disp,gc,0xFFFF00,0,GXcopy,0xFFFFFF);
    XSetFont(disp,gc,xfont->fid);*/
    // KEYS
    keys_load();
    // Load sound samples
    loadsample(SAMPLE_PRESENT,"snd/pingouin.raw", false, 1.);
    loadsample(SAMPLE_BIPINTRO,"snd/bipintro.raw", false, 1.);
    load_wave(SAMPLE_SHOT,"snd2/shot1.wav", false, 1.);
    loadsample(SAMPLE_GEAR_DN,"snd/gear_dn.raw", false, 1.);
    loadsample(SAMPLE_GEAR_UP,"snd/gear_up.raw", false, 1.);
    loadsample(SAMPLE_SCREETCH,"snd/screetch.raw", false, 1.);
    load_wave(SAMPLE_LOW_SPEED, "snd2/taxi.wav", true, .4);
    load_wave(SAMPLE_MOTOR, "snd2/spit2.wav", true, .7);
    loadsample(SAMPLE_HIT,"snd/hit.raw", false, 1.);
    loadsample(SAMPLE_MESSAGE,"snd/message.raw", false, 1.);
    load_wave(SAMPLE_EXPLOZ,"snd2/bombhit.wav", false, 1.);
    load_wave(SAMPLE_BOMB_BLAST,"snd2/boum.wav", false, 1.);
    loadsample(SAMPLE_TOLE,"snd/tole.raw", false, 1.);
    loadsample(SAMPLE_BIPBIP,"snd/bipbip.raw", false, 1.);
    loadsample(SAMPLE_BIPBIP2,"snd/bipbip2.raw", false, 1.);
    loadsample(SAMPLE_BIPBIP3,"snd/bipcarte.raw", false, 1.);
    loadsample(SAMPLE_FEU,"snd/feu.raw", true, 1.);
    loadsample(SAMPLE_TARATATA,"snd/taratata.raw", false, 1.);
    loadsample(SAMPLE_ALLELUIA,"snd/alleluia.raw", false, 1.);
    loadsample(SAMPLE_ALERT,"snd/alert.raw", false, 1.);
    loadsample(SAMPLE_PAIN,"snd/pain.raw", false, 1.);
    load_wave(SAMPLE_DEATH,"snd2/death.wav", false, 1.);
    load_wave(SAMPLE_BRAVO,"snd2/bravo.wav", false, 1.);
    // PRESENTATION
    animpresent();
    if (present() == -1) goto fin;
    affpresent(0,0);
    //
    pstr("LOADING and CREATING THE WORLD",_DY+(SY>>3)+10,0xE5D510);
    NBBOT+=NbHosts;
    playbotname = malloc(30*NbHosts);
    strcpy(&(playbotname[controled_bot])[0],myname);
    if (Dark==-1) Dark=drand48()>.9;
    Fleuve=drand48()>.01;
    /*
        Load 3D models
                           */
    LoadModeles();
    /*
        Populate world
                        */
    initworld();
    debtir = nbobj;
    printf("World is now generated (%d objs) ; let it now degenerate !\n", nbobj);
    for (i=0; i<MAX_SHOTS; i++) vieshot[i] = 0;
    for (i=0; i<MAX_DEBRIS; i++) debris[i].o = -1;
    for (i=0; i<MAX_SMOKE_SOURCES; i++) fumeesourceintens[i] = 0;
    // Camera is obj[0]
    obj[0].pos = obj[bot[controled_bot].vion].pos;
    obj[0].rot = obj[bot[controled_bot].vion].rot;
    viewed_bot = controled_bot;
    bombidx = 0;

    // effacer les tableaux de bord, les poscams et les charnières
    for (i=0; i<NBBOT; i++) {
        obj[bot[i].vion+plane_desc[bot[i].navion].tabbord].aff=0;
        obj[bot[i].vion+n_object[bot[i].navion].nbpieces-1].aff=0;
        for (j=0; j<plane_desc[bot[i].navion].nbcharngearx+plane_desc[bot[i].navion].nbcharngeary+3; j++)
            obj[bot[i].vion+j+plane_desc[bot[i].navion].nbmoyeux+1].aff=0;
    }
    // VISION
    playsound(VOICE_EXTER, SAMPLE_TARATATA, 1., &voices_in_my_head, true, false);

    /*
     * Here comes the Big Bad Loop
     */

    gtime_start();
    do {
        if (accelerated_mode) gtime_accel(MAX_DT >> 1);
        float const dt_sec = gtime_next_sec();
//      printf("dt = %f\n", dt_sec);

        struct vector v,u;
        explosion = false;
        frame_count++;

        // PJ
        control(controled_bot);

        // PNJ
        if (!game_paused) {
            // calcul les pos du sol
            for (i=0; i<NBBOT; i++) bot[i].zs=obj[bot[i].vion].pos.z-z_ground(obj[bot[i].vion].pos.x,obj[bot[i].vion].pos.y, true);
            for (i=NbHosts; i<NBBOT; i++) robot(i);
            for (i=0; i<NBTANKBOTS; i++) robotvehic(i);
            // vérifie que les playbots ne heurtent rien
            if (!Easy) {
                for (j=0; j<NbHosts; j++) if (bot[j].camp!=-1) {
                    for (i=0; i<NBBOT; i++) if (i!=j && bot[i].camp!=-1 && collision(bot[j].vion,bot[i].vion)) break;
                    if (i<NBBOT) {
                        explose(bot[i].vion, bot[j].vion);
                        explose(bot[j].vion, bot[i].vion);
                    }
                    for (i=0; i<NBZEP; i++) {
                        if (collision(bot[j].vion, zep[i].o)) {
                            explose(bot[j].vion, zep[i].o);
                            break;
                        }
                    }
                }
            }
            // message d'alerte ?
            float n;
            if (
                bot[controled_bot].camp != -1 &&    // not dead
                bot[controled_bot].vionvit.z < -.3*ONE_METER && // and fast toward the ground
                (n=bot[controled_bot].vionvit.z * 10 + bot[controled_bot].zs) < 0  // will hit ground in less than 10s
            ) {
                playsound(VOICE_ALERT, SAMPLE_ALERT, 1-n*.001, &voices_in_my_head, true, false);
            }
            // avance les shots
            for (i=debtir; i<nbobj; i++) {
                int oc;
                if (!vieshot[i-debtir]) continue;
                vieshot[i-debtir]--;
                // collision ?
                for (oc=map[obj[i].ak].first_obj; oc!=-1; oc=obj[oc].next) {
                    if (obj[oc].type != TYPE_BOMB && collision(i, oc)) {
                        if (hitgun(oc, i)) vieshot[i-debtir]=0;
                    }
                }
                v = obj[i].rot.x;
                mulv(&v, SHOT_SPEED * dt_sec);
                addv(&obj[i].pos, &v);
                obj[i].rot.x.z -= .05*dt_sec;   // FIXME: obj should have a proper velocity
                renorme(&obj[i].rot.x); // FIXME: not at every step!
                randomv(&obj[i].rot.y);
                orthov(&obj[i].rot.y,&obj[i].rot.x);
                prodvect(&obj[i].rot.x,&obj[i].rot.y,&obj[i].rot.z);
                obj_check_pos(i);
                if (vieshot[i-debtir]==0 || obj[i].pos.z<z_ground(obj[i].pos.x,obj[i].pos.y, true)) {
                    obj[i].aff=0;   // pour qu'il soit plus affiché
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
                j=bomb[i].o;
                if (j!=-1) {
                    int oc, fg=0;
                    // FIXME: given dt, &pos, &vit, &acc, drag factor, apply gravity?
                    bomb[i].vit.z -= G * dt_sec;
                    mulv(&bomb[i].vit, pow(.9, dt_sec));
                    v = bomb[i].vit;
                    mulv(&v, dt_sec);
                    addv(&obj[j].pos, &v);
                    obj_check_pos(j);
                    // collision ?
                    for (oc=map[obj[j].ak].first_obj; oc!=-1; oc=obj[oc].next)
                        if (obj[oc].type!=TYPE_SHOT && obj[oc].type!=TYPE_CAMERA && obj[oc].type!=TYPE_DECO && (oc<bot[bomb[i].b].vion || oc>=bot[bomb[i].b].vion+n_object[bot[bomb[i].b].navion].nbpieces) && collision(j,oc)) {
                            explose(oc,j);
                            fg=1;
                            break;
                        }
                    if (fg || obj[j].pos.z<z_ground(obj[j].pos.x,obj[j].pos.y, true)) {
                        playsound(VOICE_EXTER2, SAMPLE_BOMB_BLAST, 1+(drand48()-.5)*.08, &obj[j].pos, false, false);
                        obj[j].objref=bot[bomb[i].b].babase;
                        copyv(&obj[j].pos,&vec_zero);
                        copym(&obj[j].rot,&mat_id);
                        bomb[i].o=-1;
                        while (bombidx>0 && bomb[bombidx-1].o==-1) bombidx--;
                    }
                }
            }
            // avance les débris
            for (i=0; i<MAX_DEBRIS; i++) {
                if (debris[i].o!=-1) {
                    double zs;
                    v = debris[i].vit;
                    mulv(&v, dt_sec);
                    addv(&obj[debris[i].o].pos, &v);
                    float c1 = cosf(debris[i].a1);
                    float c2 = cosf(debris[i].a2);
                    float s1 = sinf(debris[i].a1);
                    float s2 = sinf(debris[i].a2);
                    obj[debris[i].o].rot.x.x = c1*c2;
                    obj[debris[i].o].rot.x.y = s1*c2;
                    obj[debris[i].o].rot.x.z = s2;
                    obj[debris[i].o].rot.y.x = -s1;
                    obj[debris[i].o].rot.y.y = c1;
                    obj[debris[i].o].rot.y.z = 0;
                    obj[debris[i].o].rot.z.x = -c1*s2;
                    obj[debris[i].o].rot.z.y = -s1*s2;
                    obj[debris[i].o].rot.z.z = c2;
                    debris[i].a1 += debris[i].ai1 * dt_sec;
                    debris[i].a2 += debris[i].ai2 * dt_sec;
                    obj_check_pos(debris[i].o);
                    mulv(&debris[i].vit, pow(.99, dt_sec));
                    debris[i].vit.z -= G * dt_sec;
                    zs=z_ground(obj[debris[i].o].pos.x,obj[debris[i].o].pos.y, true);
                    if (obj[debris[i].o].pos.z < zs) {
                        // bounce
                        obj[debris[i].o].pos.z = zs;
                        debris[i].vit.z = -0.5*debris[i].vit.z;
                        debris[i].vit.x *= 1.5*drand48();
                        debris[i].vit.y *= 1.5*drand48();
                        debris[i].ai1 = drand48()*debris[i].ai1;
                        debris[i].ai2 = drand48()*debris[i].ai2;
                        if (fabsf(debris[i].vit.z) < 30.) {
                        //  copym(&obj[debris[i].o].rot,&mat_id);
                        //  randomhm(&obj[debris[i].o].rot);
                            debris[i].o = -1;
                        }
                    }
                }
            }
            // avance la fumee
            for (fumeedispo=0, i=0; i<MAX_SMOKES; i++) {
                if (rayonfumee[i]) {
                    uchar rlim;
                    randomv(&v);
                    mulv(&v, rayonfumee[i]);
                    v.z += rayonfumee[i]>>1;
                    switch (typefumee[i]) {
                        case 0: rlim=90; break;
                        default: rlim=6; break;
                    }
                    if (++rayonfumee[i]>rlim) rayonfumee[i]=0;
                    else {
#                       define SMOKE_GROWING_SPEED (.3 * ONE_METER) // per seconds
                        mulv(&v, SMOKE_GROWING_SPEED * dt_sec);
                        addv(&obj[firstfumee+i].pos,&v);
                        obj_check_pos(firstfumee+i);
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
                            typefumee[fumeedispo]=0;    // type noir
                            copyv(&obj[firstfumee+fumeedispo].pos,&obj[bot[i].vion].pos);
                            obj_check_pos(firstfumee+fumeedispo);
                        }
                    }
                }
            }
            for (i=0; i<MAX_SMOKE_SOURCES; i++) {
                if (fumeesourceintens[i]>0) {
                    fumeesourceintens[i]--;
                    if (!(fumeesourceintens[i]&1)) {
                        for (; fumeedispo>=0 && rayonfumee[fumeedispo]; fumeedispo--);
                        if (fumeedispo>=0) {
                            rayonfumee[fumeedispo]=1;
                            copyv(&obj[firstfumee+fumeedispo].pos,&obj[fumeesource[i]].pos);
                            obj_check_pos(firstfumee+fumeedispo);
                        }
                    }
                }
            }
            // Animate cars
            for (i=0; i<NB_CARS; i++) {
                int dist;
                if (obj[car[i].o].type==TYPE_DECO) continue;
                subv3(&route[car[i].r].i,&obj[car[i].o].pos,&u);
                dist=fabs(u.x)+fabs(u.y);
                if (dist>car[i].dist) {
                    if (car[i].r<2 || car[i].r>routeidx-3 || route[car[i].r+car[i].sens].ak==-1) car[i].sens=-car[i].sens;
                    if (route[car[i].r+car[i].sens].ak!=-1) {
                        subv3(&route[car[i].r+car[i].sens].i,&route[car[i].r].i,&obj[car[i].o].rot.x);
                        renorme(&obj[car[i].o].rot.x);
                        orthov(&obj[car[i].o].rot.z,&obj[car[i].o].rot.x);
                        renorme(&obj[car[i].o].rot.z);
                        prodvect(&obj[car[i].o].rot.z,&obj[car[i].o].rot.x,&obj[car[i].o].rot.y);
                        car[i].r+=car[i].sens;
                        dist=MAXINT;
                    } else car[i].vit=0;
                }
                car[i].dist=dist;
                copyv(&v, &obj[car[i].o].rot.x);
                mulv(&v, dt_sec * car[i].vit);
                addv(&obj[car[i].o].pos, &v);
                obj_check_pos(car[i].o);
                for (j=car[i].o+1; j<car[i+1].o; j++) calcposrigide(j);
            }
            // new radio messages
            if (msgactutime) msgactutime--;
            else if (initradio || !dtradio--) {
                if (!SpaceInvaders && initradio) {
                    campactu=camp;
                    strcpy(msgactu,scenar[campactu][4-initradio][lang]);
                    initradio--;
                    playsound(VOICE_GEAR, SAMPLE_MESSAGE, 1., &voices_in_my_head, true, false);
                    msgactutime=40;
                } else {
                    newprime();
                    if (campactu==bot[viewed_bot].camp) playsound(VOICE_GEAR, SAMPLE_MESSAGE, 1., &voices_in_my_head, true, false);
                    dtradio=10+drand48()*100;
                    if (campactu==0) {
                        dtradio+=10000;
                    }
                }
            }

            // Animate mills
#           define MILL_ANGULAR_SPEED (2. * M_PI / 5.)  // one rotation every 5 secs
            AngleMoulin += (1. - 0.2*(drand48()-0.5)) * dt_sec;
            m.x.x=1; m.x.y=0; m.x.z=0;
            m.y.x=0; m.y.y=cos(AngleMoulin); m.y.z=sin(AngleMoulin);
            m.z.x=0; m.z.y=-sin(AngleMoulin); m.z.z=cos(AngleMoulin);
            for (i=DebMoulins+1; i<FinMoulins; i+=10){
                if (obj[i].type!=TYPE_DECO) {
                    calcposarti(i,&m);
                    for (j=i+1; j<i+5; j++) calcposrigide(j);
                }
            }

            for (i = 0; i < NBBOT; i++) physics_plane(i, dt_sec);
            for (i = 0; i < NBTANKBOTS; i++) physics_tank(i, dt_sec);
            for (i = 0; i < NBZEP; i++) physics_zep(i, dt_sec);

            // Now that we know the location of all objects, setup the camera.
            setup_camera(dt_sec);

            // Now that we know camera's position, play all sounds
            struct vector velocity = { 0., 0., 0. };   // FIXME
            update_listener(&obj[0].pos, &velocity, &obj[0].rot);

            // Draw the frame
            if (!accelerated_mode || 0 == (frame_count&31)) {
                // RENDU
                // La lumière vient d'où ?
                copym(&Light,&LightSol);
                if (explosion) {
                    subv3(&obj[0].pos,&explosion_pos,&u);
                    if (renorme(&u)<TILE_LEN) {
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
                if (map_mode) {
                    map_draw();
                } else {
                    background();
                    affsoleil(&Light.z);
                    mulmtv(&obj[bot[viewed_bot].vion].rot,&Light.z,&v);
                    lx=-127*v.y; ly=-127*v.z; lz=50*v.x+77;
                    if (plane_desc[bot[viewed_bot].navion].oldtb) tbback=tbback1;
                    else tbback=tbback2;
                    drawtbback();
                    drawtbcadrans(viewed_bot);
                    animate_water(dt_sec);
                    draw_ground_and_objects();
#                   ifdef VEC_DEBUG
                    draw_debug();
#                   endif
                    if (!Dark) {
                        double i;
                        uchar u;
                        if ((i=scalaire(&obj[0].rot.z,&Light.z))<-.9) {
                            u=(exp(-i-.9)-1)*2200;
                            MMXAddSatInt((int*)videobuffer,(u<<16)+(u<<8)+u,SX*SY);
                        }
                    }
                    if (Easy) {
                        if (bot[viewed_bot].cibt != -1) {
                            draw_target(obj[bot[viewed_bot].cibt].pos, 0xC02080);
                            fall_min_dist2(viewed_bot);
                            draw_mark(bot[viewed_bot].drop_mark, 0x400000);
                        }
                        if (bot[viewed_bot].cibv != -1) draw_target(obj[bot[viewed_bot].cibv].pos, 0xC08020);
                        struct vector nav = bot[viewed_bot].u;
                        nav.z += bot[viewed_bot].target_rel_alt;
                        draw_target(nav, 0x20F830);
                    }
                }
                if (view == VIEW_DOGFIGHT && bot[controled_bot].camp!=-1) cercle(0,0,10,colcamp[(int)bot[controled_bot].camp]);
                plotmouse(_DX*bot[viewed_bot].xctl,_DY*bot[viewed_bot].yctl);
#               ifdef PRINT_DEBUG
                if (bot[viewed_bot].aerobatic != MANEUVER) {
                    pstr(aerobatic_2_str(bot[viewed_bot].aerobatic), 20, 0xFF8080);
                } else {
                    pstr(maneuver_2_str(bot[viewed_bot].maneuver), 20, 0x80FF80);
                }
#               endif
                // HUD
                if (Easy) {
                    int const b = viewed_bot; // controled_bot;
                    pword("Sz:", 10, 10, 0x406040);
                    pnum(bot[b].vionvit.z, 40, 10, 0xAFDF10, 1);
                    pword("Sl:", 10, 20, 0x406040);
                    pnum(bot[b].vitlin, 40, 20, 0xFFFFFF, 1);
                    if (autopilot || b != controled_bot) {
                        float const diff_speed = bot[b].target_speed - bot[b].vitlin;
                        pnum(diff_speed, 40+4*10, 20, diff_speed > 0 ? 0xD0D0F0 : 0xF0D0D0, 1);
                    }
                    pword("St:", 10, 30, 0x406040);
                    pnum(norme(&bot[b].vionvit), 40, 30, 0x00FFFF, 1);
                    pword("Zg:", 10, 40, 0x406040);
                    pnum(bot[b].zs, 40, 40, 0xFF00FF, 1);
                    if (autopilot || b != controled_bot) {
                        float const diff_alt = (bot[b].u.z + bot[b].target_rel_alt) - obj[bot[b].vion].pos.z;
                        pnum(diff_alt, 40+4*10, 40, diff_alt > 0 ? 0xD0D0F0 : 0xF0D0D0, 1);
                    }
                    if (bot[b].but.gear) pword("gear",10,60,0xD0D0D0);
                    if (bot[b].but.flap) pword("flaps",10,70,0xD0D0D0);
                    if (bot[b].but.frein) pword("brakes",10,80,0xD0D0D0);
                    if (autopilot) pword("auto", 10, 90, 0xD0D0D0);
                }
                if (accelerated_mode) pstr("ACCELERATED MODE",_DY/3,0xFFFFFF);
                if (prompt_quit) pstr("Quit ? Yes/No",_DY/2-8,0xFFFFFF);
                if (msgactutime && bot[viewed_bot].camp==campactu) pstr(msgactu,10,0xFFFF00);
                if (view == VIEW_DOGFIGHT && bot[DogBot].camp!=-1) {
                    char vn[100];
                    snprintf(vn, sizeof(vn), "%s%s%s%s",
                        plane_desc[bot[DogBot].navion].name,
                        DogBot < NbHosts ? " (" : "",
                        DogBot < NbHosts ? playbotname[DogBot] : "",
                        DogBot < NbHosts ? ")" : "");
                    pstr(vn, SY-12, colcamp[(int)bot[DogBot].camp]);
                }
                // Display current balance
                if (bot[controled_bot].gold - 2000 > maxgold) {
                    maxgold = bot[controled_bot].gold - 2000;
                    if (maxrank < ARRAY_LEN(highscore)) highscore[maxrank].score = maxgold;
                    while (maxrank > 0 && highscore[maxrank-1].score < maxgold) {
                        maxrank--;
                        if (maxrank<ARRAY_LEN(highscore)-1) {
                            memcpy(&highscore[maxrank+1], &highscore[maxrank], sizeof(struct high_score));
                        }
                        highscore[maxrank].score = maxgold;
                        snprintf(highscore[maxrank].name, sizeof(highscore[maxrank].name), "%s", playbotname[controled_bot]);
                    }
                }
                if (bot[controled_bot].gold>oldgold) {
                    if (!caissetot && caisse>0) caisse+=bot[controled_bot].gold-oldgold;
                    else caisse=bot[controled_bot].gold-oldgold;
                    dtcaisse=20;
                    caissetot=0;
                } else if (oldgold>bot[controled_bot].gold) {
                    if (!caissetot && caisse<0) caisse+=bot[controled_bot].gold-oldgold;
                    else caisse=bot[controled_bot].gold-oldgold;
                    dtcaisse=20;
                    caissetot=0;
                }
                if (dtcaisse) {
                    pbignum(caisse,_DX,SY/3,2,caissetot,1);
                    dtcaisse--;
                    if (!dtcaisse) {
                        if (!caissetot) {
                            caisse=bot[controled_bot].gold;
                            dtcaisse=30;
                            caissetot=1;
                            playsound(VOICE_GEAR, SAMPLE_MESSAGE, 1.4, &voices_in_my_head, true, false);
                        } else {
                            caissetot=0;
                            caisse=0;
                        }
                    }
                }
                oldgold=bot[controled_bot].gold;

                if (draw_high_scores) {
                    int y=(SY-(ARRAY_LEN(highscore)+2)*9)>>1;
                    pstr("Hall of Shame",y,0xFFFFFF);
                    for (unsigned i = 0; i < ARRAY_LEN(highscore); i++) {
                        char fonom[36];
                        snprintf(fonom, sizeof(fonom), "%d. %s", highscore[i].score, highscore[i].name);
                        pstr(fonom,y+9*(2+i),i==maxrank?0xFFFF1F:0xEFD018);
                    }
                }
                plotcursor(xmouse,ymouse);
                buffer2video();
            }
        }
    } while (! quit_game);
    // FIN
fin:
    sound_fini();
    system("xset r on");    // FIXME
    // save highscores
    if (!Easy && !ViewAll && plane_desc[monvion-1].prix<=plane_desc[0].prix && (file=file_open(".fachoda-highscores", getenv("HOME"), "w+"))!=NULL) {
        fwrite(&highscore, sizeof(struct high_score), ARRAY_LEN(highscore), file);
        fclose(file);
    }
    {
        // print highscores
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
        for (unsigned i = 0; i < ARRAY_LEN(highscore); i++) {
            printf("    %2d) $%5d - %s\n", i, highscore[i].score, highscore[i].name);
        }
        i = (maxgold>2000) +
            (maxgold>4000) +
            (maxgold>6000) +
            (maxgold>8000) +
            (maxgold>10000) +
            (maxgold>12000) +
            (maxgold>14000) +
            (maxgold>16000);
        printf("\n    Your score : %d\n    %s %s.\n\n",maxgold,lang?"You retreat as a":"Vous vous retirez en tant que",rank[i][lang]);
    }
    return EXIT_SUCCESS;
}
