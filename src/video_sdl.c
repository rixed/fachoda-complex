#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/xf86dga.h>
#include <X11/Xatom.h>
#include <math.h>
#include <SDL/SDL.h>
#include "3d.h"

SDL_Surface *screen, *bufsurface;
int ncolors;
SDL_Color *colors;

int bank, size, width, BufVidOffset, yview=0, depth, XCONVERT=0;
pixel32 *videobuffer;
char *video;
GC gc;
Display *disp;
Window win,root;
XImage img = { 0,0, 0, ZPixmap,NULL, LSBFirst, 32, LSBFirst, 32, 24, 0, 24, 0,0,0 };	// dernier = 32 pour que X fasse la conversion lui meme. Mais il est si lent...
XFontStruct *xfont;

void initvideo() {
	int i;
	int r, g, b;
	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr,"Couln't initialize SDL : %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	if ((screen=SDL_SetVideoMode(SX,SY,0, SDL_SWSURFACE|SDL_ANYFORMAT|(WINDOW?0:SDL_FULLSCREEN)))==NULL) {
		fprintf(stderr,"Couldn't set video mode : %s\n",SDL_GetError());
		exit(1);
	}
	printf("Set %dx%d at %d bits-per-pixel mode\n",screen->w,screen->h,screen->format->BitsPerPixel);
	if ((bufsurface=SDL_CreateRGBSurfaceFrom((void*)videobuffer,SX,SY,32,SX*4,0xFF0000,0xFF00,0xFF,0))==NULL) {
		fprintf(stderr,"Couldn't get surface : %s\n",SDL_GetError());
		exit(1);
	}
	ncolors = 256;
	if ((colors=(SDL_Color *)malloc(ncolors*sizeof(SDL_Color)))==NULL) {
		perror("malloc color"); exit(-1);
	}
	for (r=0; r<8; ++r) {
		for (g=0; g<8; ++g ) {
			for (b=0; b<4; ++b ) {
				i = ((r<<5)|(g<<2)|b);
				colors[i].r = r<<5;
				colors[i].g = g<<5;
				colors[i].b = b<<6;
			}
		}
	}
	SDL_SetColors(screen, colors, 0, ncolors);
	free(colors);
	SDL_ShowCursor(0);
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTIONMASK, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);

	SDL_WM_SetCaption("FACHODA Complex","FACHODA Complex");
}

void buffer2video() {
	SDL_Rect dstrect;
/*	if (!WINDOW) MMXCopyToScreen((int*)video,(int*)videobuffer,SX,SY,width);
	else {
		if (!XCONVERT && depth!=32) MMXCopyToScreen((int*)img.data,(int*)videobuffer,SX,SY,SX);
		XPutImage(disp, win, gc, &img, 0,0, 0,0, SX,SY);*/
		dstrect.x=dstrect.y=0;
		dstrect.w=bufsurface->w;
		dstrect.h=bufsurface->h;
		if(SDL_BlitSurface(bufsurface,NULL,screen,&dstrect)<0) {
			fprintf(stderr,"Couldn't blit : %s\n",SDL_GetError());
		}
		SDL_UpdateRects(screen,1,&dstrect);
//	}
}

char keytab[32]={
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};
uchar but1released=1, but2released=1;
int xmouse,ymouse,bmouse;
inline void bitset(int n) {__asm__ ("bts %0,keytab": :"r" (n): "memory");}
inline void bitzero(int n) {__asm__ ("btr %0,keytab": :"r" (n): "memory");}

void xproceed() {
	SDL_Event event;
	bmouse=SDL_GetMouseState(&xmouse,&ymouse);
	if (xmouse<0) xmouse=0;
	if (ymouse<0) ymouse=0;
	if (xmouse>=SX) xmouse=SX-1;
	if (ymouse>=SY) ymouse=SY-1;
	if (bmouse&SDL_BUTTON(1) && but1released) {
		bitset(0);
		but1released=0;
	}
	if (bmouse&SDL_BUTTON(3) && but2released) {
		bitset(1);
		but2released=0;
	}
	if (!(bmouse&SDL_BUTTON(1))) {
		but1released=1;
		bitzero(0);
	}
	if (!(bmouse&SDL_BUTTON(3))) {
		but2released=1;
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
	return -1;	// ??
}
