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
#include <math.h>
#include "proto.h"
#include "heightfield.h"
#include "keycodesdef.h"
#include "sound.h"
#include "gtime.h"
#include "robot.h"

int DogBot=0;
struct vector DogBotDir;
float DogBotDist;

static int resurrect(void)
{
    int j=NBBOT, jj, bestprix=0;
    struct bot bottmp;
    for (jj=NbHosts; jj<NBBOT; jj++) {
        if (bot[jj].camp==camp && plane_desc[bot[jj].navion].prix<=plane_desc[bot[controled_bot].navion].prix && plane_desc[bot[jj].navion].prix>bestprix) {
            bestprix=plane_desc[bot[jj].navion].prix;
            j=jj;
        }
    }
    if (j<NBBOT) {
        memcpy(&bottmp,&bot[controled_bot],sizeof(struct bot));
        memcpy(&bot[controled_bot],&bot[j],sizeof(struct bot));
        memcpy(&bot[j],&bottmp,sizeof(struct bot));
        bot[controled_bot].gold=55;
        playsound(VOICE_EXTER, SAMPLE_ALLELUIA, 1., &voices_in_my_head, true, false);
        snd_thrust=-1;
        autopilot = true;
        accelerated_mode = false;
        map_x=obj[bot[controled_bot].vion].pos.x/TILE_LEN;
        map_y=obj[bot[controled_bot].vion].pos.y/TILE_LEN;
        return 1;
    }
    return 0;
}

void next_dog_bot(void)
{
    int DBi=DogBot;
    DogBotDist=0;
    do {
        if (++DogBot>=NBBOT) DogBot=0;
        if (DogBot!=controled_bot) {
            copyv(&DogBotDir,&obj[bot[DogBot].vion].pos);
            subv(&DogBotDir,&obj[bot[controled_bot].vion].pos);
            DogBotDist=renorme(&DogBotDir);
        }
    } while (DogBot!=DBi && (DogBotDist>DOGDISTMAX || bot[DogBot].camp==-1 || DogBot==controled_bot));
}

static void prev_dog_bot(void)
{
    int DBi=DogBot;
    DogBotDist=0;
    do {
        if (--DogBot<0) DogBot=NBBOT-1;
        if (DogBot!=controled_bot) {
            copyv(&DogBotDir,&obj[bot[DogBot].vion].pos);
            subv(&DogBotDir,&obj[bot[controled_bot].vion].pos);
            DogBotDist=renorme(&DogBotDir);
        }
    } while (DogBot!=DBi && (DogBotDist>DOGDISTMAX || bot[DogBot].camp==-1 || DogBot==controled_bot));
}

enum view_type next_external_view(enum view_type v)
{
    if (v < VIEW_ROTATING_PLANE) {
        return VIEW_ROTATING_PLANE;
    }
    if (v == NB_VIEWS-1) return VIEW_ROTATING_PLANE;
    return v+1;
}

void control(int b)
{
    int i;
    struct vector u;
    xproceed();
    // Left button
    if (! map_mode) switch (selected_weapon) {
    case 0:
        if ((enable_mouse && kread(0)) || kread(gkeys[kc_fire].kc)) bot[b].but.canon=1;
        break;
    case 1:
        if ((enable_mouse && kreset(0)) || kreset(gkeys[kc_fire].kc)) bot[b].but.bomb=1;
        break;
    } else if ((enable_mouse && kread(0)) || kread(gkeys[kc_fire].kc)) {
        bot[b].u.x = ((xmouse-win_center_x)*(MAP_LEN/2)*TILE_LEN)/zoom+map_x*TILE_LEN;
        bot[b].u.y = ((win_center_y-ymouse)*(MAP_LEN/2)*TILE_LEN)/zoom+map_y*TILE_LEN;
        bot[b].u.z = z_ground(bot[b].u.x, bot[b].u.y, true);
    }
    // Right button
    if ((enable_mouse && kreset(1)) || kreset(gkeys[kc_weapon].kc)) {
        if (abs(xmouse) < 2 && abs(ymouse) < 2) {
            if (! prompt_quit) prompt_quit = true;
            else quit_game = true;
        }
        selected_weapon ^= 1;
    }
    // Esc
    if (prompt_quit) {
        if (kreset(gkeys[kc_yes].kc)) quit_game = true;
        if (kreset(gkeys[kc_no].kc)) prompt_quit = false;
    } else if (kreset(gkeys[kc_esc].kc) && (bot[controled_bot].camp!=-1 || !enable_resurrection || !resurrect())) {
        prompt_quit = true;
    }
    // Motor
    if (kread(gkeys[kc_motormore].kc)) bot[b].thrust+=.05;
    if (kread(gkeys[kc_motorless].kc)) bot[b].thrust-=.05;
    // Views
    if (kreset(gkeys[kc_externview].kc)) {
        map_mode = false;
        view = next_external_view(view);
        snd_thrust=-1;
    }
    if (kreset(gkeys[kc_internview].kc)) {
        map_mode = false;
        if (view == VIEW_IN_PLANE) {
            view = VIEW_DOGFIGHT;
        } else {
            view = VIEW_IN_PLANE;
            snd_thrust=-1;
        }
    }
    if (kreset(gkeys[kc_travelview].kc)) {
        float zs;
        map_mode = false;
        view = VIEW_STANDING;
        copyv(&obj[0].pos,&obj[bot[viewed_bot].vion].rot.x);
        mulv(&obj[0].pos,300+drand48()*600+extcam_dist);
        copyv(&u,&obj[bot[viewed_bot].vion].rot.y);
        mulv(&u,(drand48()-.5)*600);
        addv(&obj[0].pos,&u);
        copyv(&u,&obj[bot[viewed_bot].vion].rot.z);
        mulv(&u,(drand48()-.5)*600);
        addv(&obj[0].pos,&u);
        addv(&obj[0].pos,&obj[bot[viewed_bot].vion].pos);
        if (obj[0].pos.z<(zs=z_ground(obj[0].pos.x,obj[0].pos.y, false)+100)) obj[0].pos.z=zs;
        snd_thrust=-1;
    }
    if (kreset(gkeys[kc_nextbot].kc)) {
        if (view == VIEW_ANYTHING_CHEAT) {
            if (++viewed_obj >= nb_obj) viewed_obj = 0;
        } else if (view == VIEW_DOGFIGHT) {
            next_dog_bot();
        } else {
            do {
                if (++viewed_bot>=NBBOT) viewed_bot=0;
            } while (!enable_view_enemy && bot[viewed_bot].camp!=camp);  // pas controled_bot.camp car peut etre tue
            snd_thrust=-1;
            if (bot[viewed_bot].camp==-1) playsound(VOICE_MOTOR, SAMPLE_FEU, 1., &voices_in_my_head, true, true);
        }
    }
    if (kreset(gkeys[kc_prevbot].kc)) {
        if (view == VIEW_ANYTHING_CHEAT) {
            if (--viewed_obj<0) viewed_obj = nb_obj-1;
        } else if (view == VIEW_DOGFIGHT) {
            prev_dog_bot();
        } else {
            do {
                if (--viewed_bot<0) viewed_bot=NBBOT-1;
            } while (!enable_view_enemy && bot[viewed_bot].camp!=camp);
            snd_thrust=-1;
            if (bot[viewed_bot].camp==-1) playsound(VOICE_MOTOR, SAMPLE_FEU, 1., &voices_in_my_head, true, true);
        }
    }
    if (kreset(gkeys[kc_mybot].kc)) {
        if (view != VIEW_DOGFIGHT) {
            viewed_bot=b;
            snd_thrust=-1;
        } else {
            float d;
            int DBi, DBm;
            next_dog_bot();
            d=DogBotDist;
            DBi=DogBot; DBm=DogBot;
            do {
                next_dog_bot();
                if (DogBotDist<d && bot[DogBot].camp!=bot[controled_bot].camp) {
                    d=DogBotDist;
                    DBm=DogBot;
                }
            } while (DogBot!=DBi);
            DogBot=DBm; DogBotDist=d;
        }
    }
    if (kreset(gkeys[kc_nextbomb].kc)) {
        for (
            i = bot[viewed_bot].vion;
            i < bot[viewed_bot].vion + n_object[bot[viewed_bot].navion].nbpieces &&
            (obj[i].objref != -1 || (obj[i].type != TYPE_BOMB));
            i++
        ) ;
        if (i < bot[viewed_bot].vion + n_object[bot[viewed_bot].navion].nbpieces) {
            viewed_bomb = i;
        } else {
            viewed_bomb = 0;
        }
    }
    if (!accelerated_mode || frame_count > 64) {
            if (kread(gkeys[kc_zoomout].kc)) {
            if (! map_mode) extcam_dist += 10;
            else zoom += win_center_x/6;
        }
        if (kread(gkeys[kc_zoomin].kc)) {
            if (! map_mode) extcam_dist -= 10;
            else if ((zoom -= win_center_x/6) < win_center_x) zoom = win_center_x;
        }
        if (kread(gkeys[kc_riseview].kc)) {
            if (! map_mode) {
                if ((sight_teta -= .2) < -M_PI) sight_teta += 2*M_PI;
            } else if ((map_y += 1 + (3*win_width)/zoom) > MAP_LEN/2) {
                map_y = MAP_LEN/2;
            }
        }
        if (kread(gkeys[kc_lowerview].kc)) {
            if (! map_mode) {
                if ((sight_teta += .2) > M_PI) sight_teta -= 2*M_PI;
            } else if ((map_y -= 1 + (3*win_width)/zoom) < -MAP_LEN/2) {
                map_y = -MAP_LEN/2;
            }
        }
        if (kread(gkeys[kc_rightenview].kc)) {
            if (! map_mode) {
                if ((sight_phi -= .2) < -M_PI) sight_phi += 2*M_PI;
            } else if ((map_x += 1 + (3*win_width)/zoom) > MAP_LEN/2) {
                map_x = MAP_LEN/2;
            }
        }
        if (kread(gkeys[kc_leftenview].kc)) {
            if (! map_mode) {
                if ((sight_phi += .2) > M_PI) sight_phi -= 2*M_PI;
            } else if ((map_x -= 1 + (3*win_width)/zoom) < -MAP_LEN/2) {
                map_x = -MAP_LEN/2;
            }
        }
    }
    if (view != VIEW_DOGFIGHT) {
        if (kreset(gkeys[kc_towardview].kc)) { sight_teta = sight_phi = 0; }
        if (kreset(gkeys[kc_backview].kc)) { sight_teta = 0; sight_phi = M_PI; }
        if (kreset(gkeys[kc_leftview].kc)) { sight_teta = 0; sight_phi = M_PI*.5; }
        if (kreset(gkeys[kc_rightview].kc)) { sight_teta = 0; sight_phi = -M_PI*.5; }
        if (kreset(gkeys[kc_upview].kc)) { sight_teta = -M_PI/2; sight_phi = 0; }
    } else {
        view_predef = false;
        if (kread(gkeys[kc_towardview].kc)) { view_predef = true; sight_teta = sight_phi = 0; }
        if (kread(gkeys[kc_backview].kc)) { view_predef = true; sight_teta = 0; sight_phi = M_PI; }
        if (kread(gkeys[kc_leftview].kc)) { view_predef = true; sight_teta = 0; sight_phi = M_PI*.5; }
        if (kread(gkeys[kc_rightview].kc)) { view_predef = true; sight_teta = 0; sight_phi = -M_PI*.5; }
        if (kread(gkeys[kc_upview].kc)) { view_predef = true; sight_teta = -M_PI/2; sight_phi = 0; }
        if (! view_predef) sight_teta = sight_phi = 0;
    }
    view_instruments = kread(gkeys[kc_movetowardview].kc);
    // Commands
    if (kreset(gkeys[kc_gear].kc)) bot[b].but.gear^=1;
    if (kreset(gkeys[kc_flaps].kc)) {
        bot[b].but.flap^=1;
        playsound(VOICE_GEAR, SAMPLE_BIPBIP, 1., &obj[bot[b].vion].pos, false, false);
    }
    bot[b].but.frein=kread(gkeys[kc_brakes].kc);
    if (kreset(gkeys[kc_business].kc)) bot[b].but.business = 1;
    if (kreset(gkeys[kc_autopilot].kc)) {
        autopilot = ! autopilot;
        playsound(VOICE_GEAR, SAMPLE_BIPBIP, 1., &obj[bot[b].vion].pos, false, false);
        if (autopilot) {
            bot[controled_bot].target_speed = BEST_LIFT_SPEED;
            bot[controled_bot].target_rel_alt = 100. * ONE_METER;
        }
    }
    // Game control
    if (kreset(gkeys[kc_pause].kc)) {
        gtime_toggle();
        game_paused = ! game_paused;
    }
    draw_high_scores = kread(gkeys[kc_highscores].kc);
    if (kreset(gkeys[kc_accelmode].kc)) { accelerated_mode = ! accelerated_mode; frame_count&=63; }
    if (kreset(gkeys[kc_basenav].kc)) {
        bot[b].u = obj[bot[b].babase].pos;
    }
    if (kreset(gkeys[kc_mapmode].kc)) {
        map_mode = ! map_mode;
        playsound(VOICE_GEAR, SAMPLE_BIPBIP3, 1., &voices_in_my_head, true, false);
    }
    if (kreset(gkeys[kc_suicide].kc) && bot[controled_bot].camp!=-1) explose(bot[viewed_bot].vion, 0);
    if (kreset(gkeys[kc_markpos].kc)) bot[b].but.mark=1;
    // Cheats
    if (cheat_mode && kread(gkeys[kc_alti].kc)) {
        obj[bot[viewed_bot].vion].pos.z += 500;
        bot[viewed_bot].vionvit.z = 0;
    }
    if (cheat_mode && kreset(gkeys[kc_gunned].kc)) bot[viewed_bot].gunned=controled_bot;
    if (!autopilot && !map_mode) {
        if (enable_mouse) {
            bot[b].xctl = ((xmouse-win_center_x)/(double)win_center_x);
            bot[b].yctl = ((ymouse-win_center_y)/(double)win_center_y);
        } else {
            int i=0;
            i=kread(gkeys[kc_left].kc);
            i+=kread(gkeys[kc_right].kc)<<1;
            i+=kread(gkeys[kc_down].kc)<<2;
            i+=kread(gkeys[kc_up].kc)<<3;
            if (i) {
                CtlSensActu += CtlSensitiv;
                if (i&1) bot[b].xctl-=CtlSensActu;
                if (i&2) bot[b].xctl+=CtlSensActu;
                if (i&4) bot[b].yctl-=CtlSensActu;
                if (i&8) bot[b].yctl+=CtlSensActu;
            } else CtlSensActu=0;
            if (bot[b].xctl<-1 || bot[b].xctl>1 || bot[b].yctl<-1 || bot[b].yctl>1) CtlSensActu=0;
            if (!(i&3)) bot[b].xctl*=CtlAmortis;
            if (!(i&12)) bot[b].yctl=CtlYequ+(bot[b].yctl-CtlYequ)*CtlAmortis;
            if (kread(gkeys[kc_center].kc)) {
                bot[b].xctl=0;
                bot[b].yctl=CtlYequ;
                if (kread(gkeys[kc_down].kc) && CtlYequ>-1) CtlYequ-=.02;
                if (kread(gkeys[kc_up].kc) && CtlYequ<1) CtlYequ+=.02;
            }
        }
    } else {    // autopilot or map_mode
        if (autopilot) {
            robot_autopilot(b);
        } else {
            robot_safe(b, SAFE_LOW_ALT);
        }
    }
    CLAMP(bot[b].xctl, 1.);
    CLAMP(bot[b].yctl, 1.);
}
