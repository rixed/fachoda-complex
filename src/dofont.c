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
int SX=8*16,SY=10*7+1;

void main() {
	char **fontname;
	int i,u,x,y;
	img.data=malloc(SX*SY*4);
	img.width=width=SX; img.height=SY;
	disp = XOpenDisplay("");
	gc = DefaultGC(disp,DefaultScreen(disp));
	root = DefaultRootWindow(disp);
	win=XCreateSimpleWindow(disp, root, 0,0, SX,SY, 0,0,15);
	XSelectInput(disp,win,ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask);
	XInitImage(&img);
	XPutImage(disp, win, gc, &img, 0,0, 0,0, SX,SY);
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
