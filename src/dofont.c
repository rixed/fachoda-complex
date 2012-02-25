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
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86dga.h>
int bank, size, width, BufVidOffset, yview=0, xmouse, ymouse, bmouse;
GC gc;
Display *disp;
Window win,root;
XEvent  ev;
XImage img = { 0,0, 0, ZPixmap,NULL, LSBFirst, 32, LSBFirst, 32, 24, 0, 32, 0,0,0 };
XFontStruct *xfont;
int win_width=8*16,win_height=10*7+1;

void main() {
    char **fontname;
    int i,u,x,y;
    img.data=malloc(win_width*win_height*4);
    img.width=width=win_width; img.height=win_height;
    disp = XOpenDisplay("");
    gc = DefaultGC(disp,DefaultScreen(disp));
    root = DefaultRootWindow(disp);
    win=XCreateSimpleWindow(disp, root, 0,0, win_width,win_height, 0,0,15);
    XSelectInput(disp,win,ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask);
    XInitImage(&img);
    XPutImage(disp, win, gc, &img, 0,0, 0,0, win_width,win_height);
    XMapWindow(disp,win);

    fontname=XListFonts(disp,"-*-clean-*-*-*-*-*-*-*-*-*-*-*-*",1,&i);
    xfont=XLoadQueryFont(disp,fontname[0]);
    XFreeFontNames(fontname);
    XSetState(disp,gc,0xFFFF00,0,GXcopy,0xFFFFFF);
    XSetFont(disp,gc,xfont->fid);

    for (u=0, y=0; y<7; y++)
        for (x=0; x<16; x++, u++) {
            char let;
            let=u+16;
            XDrawString(disp,win,gc,x*8,y*10+9,&let,1);
        }
    while (1) if (XPending(disp)) XNextEvent(disp, &ev);
}
