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
#include <stdint.h>
#include <assert.h>
#include "proto.h"
#include "video_sdl.h"

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

int xmouse, ymouse;
static bool mouse_button[7];
static uint8_t keytab[SDLK_LAST-SDLK_FIRST+1];

static unsigned bit_of_key(SDLKey k)
{
#   if SDLK_FIRST > 0
    assert(k >= SDLK_FIRST);
#   endif
    assert(k <= SDLK_LAST);
    return k - SDLK_FIRST;
}

static void bitset(unsigned n)
{
    keytab[n/8] |= 1U<<(n&7);
}

static void bitzero(unsigned n)
{
    keytab[n/8] &= ~(1U<<(n&7));
}

static bool bittest(unsigned n)
{
    return !!(keytab[n/8] & (1U<<(n&7)));
}

bool kread(SDLKey k)
{
    return bittest(bit_of_key(k));
}

bool kreset(SDLKey k)
{
    unsigned n = bit_of_key(k);
    bool const r = bittest(n);
    bitzero(n);
    return r;
}

bool button_read(unsigned b)
{
    assert(b < ARRAY_LEN(mouse_button));
    return mouse_button[b];
}

bool button_reset(unsigned b)
{
    assert(b < ARRAY_LEN(mouse_button));
    bool r = mouse_button[b];
    mouse_button[b] = false;
    return r;
}

void xproceed(void)
{
    SDL_Event event;

    // Pointer position
    SDL_GetMouseState(&xmouse, &ymouse);
    if (xmouse < 0) xmouse = 0;
    if (ymouse < 0) ymouse = 0;
    if (xmouse >= win_width) xmouse = win_width-1;
    if (ymouse >= win_height) ymouse = win_height-1;

    // Keys
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                bitset(bit_of_key(event.key.keysym.sym));
                break;
            case SDL_KEYUP:
                bitzero(bit_of_key(event.key.keysym.sym));
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button < ARRAY_LEN(mouse_button)) {
                    mouse_button[event.button.button] = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button < ARRAY_LEN(mouse_button)) {
                    mouse_button[event.button.button] = false;
                }
                break;
            case SDL_QUIT:
                quit_game = true;
                break;
        }
    }
}

SDLKey getscancode(void)
{
    SDL_Event event;
    while (SDL_WaitEvent(&event) >= 0) {
        switch (event.type) {
            case SDL_KEYDOWN:
                return event.key.keysym.sym;
            case SDL_QUIT:
                exit(0);
        }
    }
    assert(!"Error in SDL_WaitEvent()");
    return -1;  // ??
}
