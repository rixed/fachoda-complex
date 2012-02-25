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
#include <math.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include "proto.h"

static SDL_Surface *screen, *bufsurface;
struct pixel32 *videobuffer;

void initvideo(bool fullscreen)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr,"Couln't initialize SDL : %s\n",SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(win_width, win_height, 0, SDL_SWSURFACE|SDL_ANYFORMAT|(fullscreen?SDL_FULLSCREEN:0));
    if (! screen) {
        fprintf(stderr,"Couldn't set video mode : %s\n",SDL_GetError());
        exit(1);
    }
    printf("Set %dx%d at %d bits-per-pixel mode\n",screen->w,screen->h,screen->format->BitsPerPixel);

    bufsurface = SDL_CreateRGBSurfaceFrom((void*)videobuffer, win_width, win_height, 32, win_width*4, 0xFF0000, 0xFF00, 0xFF, 0);
    if (! bufsurface) {
        fprintf(stderr,"Couldn't get surface : %s\n",SDL_GetError());
        exit(1);
    }

    SDL_ShowCursor(0);
    SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEMOTIONMASK, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);

    SDL_WM_SetCaption("FACHODA Complex","FACHODA Complex");
}

void buffer2video(void)
{
    SDL_Rect dstrect;
    dstrect.x=dstrect.y=0;
    dstrect.w=bufsurface->w;
    dstrect.h=bufsurface->h;
    if(SDL_BlitSurface(bufsurface,NULL,screen,&dstrect)<0) {
        fprintf(stderr,"Couldn't blit : %s\n",SDL_GetError());
    }
    SDL_UpdateRects(screen,1,&dstrect);
}

char keytab[32];

int xmouse, ymouse;

static void bitset(unsigned n)
{
    keytab[n/8] |= 1U<<(n&7);
}

static void bitzero(unsigned n)
{
    keytab[n/8] &= ~(1U<<(n&7));
}

bool kread(unsigned n)
{
    return !!(keytab[n/8] & (1U<<(n&7)));
}

bool kreset(unsigned n)
{
    bool r = kread(n);
    bitzero(n);
    return r;
}

void xproceed(void)
{
    static bool but1released = true, but2released = true;
    SDL_Event event;
    int bmouse = SDL_GetMouseState(&xmouse,&ymouse);
    if (xmouse<0) xmouse=0;
    if (ymouse<0) ymouse=0;
    if (xmouse>=win_width) xmouse=win_width-1;
    if (ymouse>=win_height) ymouse=win_height-1;
    if (bmouse&SDL_BUTTON(1) && but1released) {
        bitset(0);
        but1released = false;
    }
    if (bmouse&SDL_BUTTON(3) && but2released) {
        bitset(1);
        but2released = false;
    }
    if (!(bmouse&SDL_BUTTON(1))) {
        but1released = true;
        bitzero(0);
    }
    if (!(bmouse&SDL_BUTTON(3))) {
        but2released = true;
        bitzero(1);
    }
    while (SDL_PollEvent(&event)) {
        if (event.type==SDL_KEYDOWN) bitset(event.key.keysym.scancode);
        else if (event.type==SDL_KEYUP) bitzero(event.key.keysym.scancode);
    }
}

char getscancode() {
    SDL_Event event;
    while (SDL_WaitEvent(&event)>=0) {
        if (event.type==SDL_KEYDOWN) return event.key.keysym.scancode;
    }
    return -1;  // ??
}
