#include <sys/types.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/xf86dga.h>
#include <X11/Xatom.h>
#include <math.h>
#include "3d.h"

int bank, size, width, BufVidOffset, yview=0, depth, XCONVERT=0;
pixel32 *videobuffer;
char *video;
GC gc;
Display *disp;
Window win,root;
XImage img = { 0,0, 0, ZPixmap,NULL, LSBFirst, 32, LSBFirst, 32, 24, 0, 24, 0,0,0 };	// dernier = 32 pour que X fasse la conversion lui meme. Mais il est si lent...
XFontStruct *xfont;

void initdepth() {
	int i;
	XPixmapFormatValues *xpfv;
	if ((xpfv=XListPixmapFormats(disp,&i))!=NULL) {
		depth=xpfv[i-1].bits_per_pixel;
		XFree(xpfv);
	} else {
		printf("XListPixmapFormats bizare error\n");
		exit(-1);
	}
}
void deletecursor() {
	Cursor cursor;
	Pixmap f, m;
	struct {
		int		width;
		int		hot[2];
		unsigned char	mask[64];
	} c = {
		18,
		{8, 8},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	};
	XColor bl, wh;
	f=XCreatePixmapFromBitmapData(disp,root,(char*)c.mask,c.width,c.width,1,0,1);
	m=XCreatePixmapFromBitmapData(disp,root,(char*)c.mask,c.width,c.width,1,0,1);
	cursor=XCreatePixmapCursor(disp,f,m,&bl,&wh,c.hot[0],c.hot[1]);
	XDefineCursor(disp, win, cursor);
}
void fadeout() {
	int dx,dy,y;//,y0,c;
//	pixel *vid,*vidtmp,*vidt;
	XF86DGAGetViewPortSize(disp,0,&dx,&dy);
	for (y=0; y<dy; y++) MMXMemSetInt((int*)video+((width*depth*y)>>5),0,(dx*depth)>>5);
	printf("DEPTH=%d\nWIDTH=%d\n",depth,width);
	video+=((dx-SX)>>1)*(depth>>3)+(width*((dy-SY)>>1))*(depth>>3);
}
void	InitDGA () {
	XSetWindowAttributes xswa;
	XSizeHints *x_hints;

// open the display
	disp = XOpenDisplay("");
	root = DefaultRootWindow(disp);
	initdepth();
	XSynchronize(disp, True);

// use the root window for everything
//	win = RootWindow(disp, DefaultScreen(disp));
	xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;

	win = XCreateWindow(disp, DefaultRootWindow(disp), 0, 0,
		WidthOfScreen(ScreenOfDisplay(disp, 0)),
		HeightOfScreen(ScreenOfDisplay(disp, 0)), 0,
		CopyFromParent, InputOutput, CopyFromParent,
		CWEventMask, &xswa);
	XSetTransientForHint(disp, win, win);
	x_hints = XAllocSizeHints();
	x_hints->flags = USPosition;
	XSetWMNormalHints(disp, win, x_hints);
//	X_Free(x_hints);
	XMapWindow(disp, win);
	XRaiseWindow(disp, win);

// set up the mode and get info on the fb
	XF86DGAGetVideo(disp, DefaultScreen(disp), (char**)&video, &width, &bank, &size);
	XF86DGADirectVideo(disp, DefaultScreen(disp), XF86DGADirectGraphics/*|XF86DGADirectMouse*/|XF86DGADirectKeyb);
	setuid(getuid());

	XF86DGASetViewPort(disp, DefaultScreen(disp), 0, 0);
	XF86DGASetVidPage(disp, DefaultScreen(disp), 0);
// make input rawer
	XAutoRepeatOff(disp);
	XGrabKeyboard(disp, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	XGrabPointer(disp, win, True, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	XWarpPointer(disp,None,root,0,0,0,0,SX>>1,SY>>1);
	fadeout();
}
void initXwin(){
	XSetWindowAttributes xswa;
	int screen;
//	XPixmapFormatValues *xpfv;
	img.width=width=SX; img.height=SY;
	disp = XOpenDisplay("");
	gc = DefaultGC(disp,DefaultScreen(disp));
	root = DefaultRootWindow(disp);
	screen = DefaultScreen(disp);
	initdepth();
	if (!XCONVERT && depth!=32) img.data=malloc(SX*SY*(depth>>3));
	else img.data=(char *)videobuffer;
	img.depth=DefaultDepth(disp,screen);
	img.bits_per_pixel=XCONVERT?32:depth;	// on fait nous meme la conversion
//	win=XCreateSimpleWindow(disp, root, 0,0, SX,SY, 0,0,15);
	XSynchronize(disp, False);
	xswa.event_mask=KeyPressMask|KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;
	win=XCreateWindow(disp, DefaultRootWindow(disp), 0,0,SX,SY,0,CopyFromParent,InputOutput, CopyFromParent, CWEventMask, &xswa);
	deletecursor();
	XChangeProperty(disp, win, XA_WM_NAME, XA_STRING, 8, PropModeReplace, (unsigned char*)"Fachoda complex", 15);
//	XSelectInput(disp,win,ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask);
	XInitImage(&img);
	XPutImage(disp, win, gc, &img, 0,0, 0,0, SX,SY);
	XMapWindow(disp,win);
	XAutoRepeatOff(disp);
}

void buffer2video() {
	if (!WINDOW) MMXCopyToScreen((int*)video,(int*)videobuffer,SX,SY,width);
	else {
		if (!XCONVERT && depth!=32) MMXCopyToScreen((int*)img.data,(int*)videobuffer,SX,SY,SX);
		XPutImage(disp, win, gc, &img, 0,0, 0,0, SX,SY);
	}
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
	Window root_return,child_return;
	int root_x_return,root_y_return;
	XEvent ev;
	
	XQueryPointer(disp,win,&root_return,&child_return,&root_x_return,&root_y_return,&xmouse,&ymouse,&bmouse);
	if (xmouse<0) xmouse=0;
	if (ymouse<0) ymouse=0;
	if (xmouse>=SX) xmouse=SX-1;
	if (ymouse>=SY) ymouse=SY-1;
	if (bmouse&Button1Mask && but1released) {
		bitset(0);
		but1released=0;
	}
	if (bmouse&Button3Mask && but2released) {
		bitset(1);
		but2released=0;
	}
	if (!(bmouse&Button1Mask)) {
		but1released=1;
		bitzero(0);
	}
	if (!(bmouse&Button3Mask)) {
		but2released=1;
		bitzero(1);
	}
	if (XPending(disp)) {
		XNextEvent(disp, &ev);
		if (ev.type==KeyPress) {
			bitset(ev.xkey.keycode);
		} else if (ev.type==KeyRelease) bitzero(ev.xkey.keycode);
	}
}

void initvideo() {
	if (!WINDOW) InitDGA();
	else initXwin();
	if (depth!=24 && depth!=16 && depth!=32) {
		printf("Manual conversion only works on 16, 24 or 32 bits !\n");
		exit(-1);
	}
}

char getscancode() {
	XEvent ev;
	while (1) if (XPending(disp)) {
		XNextEvent(disp, &ev);
		if (ev.type==KeyPress) return ev.xkey.keycode;
	}
}
