#include <ClanLib/core.h>

extern "C" int SX,SY;

char keytab[32]={
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

int XCONVERT=0, BufVidOffset;
struct pixel32 {
	unsigned char b,g,r,u;
};
pixel32 *videobuffer;
unsigned char but1released=1, but2released=1;
int xmouse,ymouse,bmouse;
inline void bitset(int n) {__asm__ ("bts %0,keytab": :"r" (n): "memory");}
inline void bitzero(int n) {__asm__ ("btr %0,keytab": :"r" (n): "memory");}

extern "C" int mymain(int, char**);

CL_Canvas *canvas;
CL_Surface *win;

class MyClanLibApplication : public CL_ClanApplication {
	public:
		virtual char *get_title() {
			return "Fachoda complex";
		}
		virtual void init_modules() {
			CL_SetupCore::init();
		}
		virtual void deinit_modules() {
			CL_SetupCore::deinit();
		}
		virtual int main(int argc, char **argv) {
			CL_SetupCore::init();
			CL_SetupCore::init_display();
			
			
			int ret = mymain(argc,argv);
			delete canvas;
			delete win;
			return ret;
		}
} app;

extern "C" void initvideo() {
	CL_Display::set_videomode(SX,SY,32, false, false);	// no fulscreen, disalow resize
	canvas = new CL_Canvas::CL_Canvas(SX,SY,1,0xFF0000,0xFF00,0xFF,0xFF000000);
	videobuffer = (pixel32*)canvas->get_data();
	win = CL_Surface::create_dynamic(canvas);
	canvas->lock();
}
extern "C" void buffer2video() {
	canvas->unlock();
	win->reload();
	win->put_screen(0,0);
	canvas->lock();
	CL_Display::flip_display();
}
extern "C" void xproceed() {
	CL_System::keep_alive();
	xmouse = CL_Mouse::get_x();
	ymouse = CL_Mouse::get_y();
	if (xmouse<0) xmouse=0;
	if (ymouse<0) ymouse=0;
	if (xmouse>=SX) xmouse=SX-1;
	if (ymouse>=SY) ymouse=SY-1;
	if (CL_Mouse::left_pressed() && but1released) {
		bitset(0);
		but1released=0;
	}
	if (CL_Mouse::right_pressed() && but2released) {
		bitset(1);
		but2released=0;
	}
	if (!(CL_Mouse::left_pressed())) {
		but1released=1;
		bitzero(0);
	}
	if (!(CL_Mouse::right_pressed())) {
		but2released=1;
		bitzero(1);
	}
}
extern "C" char getscancode() {
	return 0;
}

