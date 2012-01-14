OBJET=renderer.o map.o modele.o radio.o robot.o tableaubord.o txt.o ran0.o control.o mapping.o manuel.o soleil.o carte.o sound.o ravages.o present.o net.o keycodes.o route.o drawroute.o init.o intro.o
MAIN=naw.o
V_SDL=video_sdl.o
OBJ2=codepeindre.o
P2=peindre.o
CPP=/usr/local/pgcc/bin/g++

CFLAGS += -std=c99 -D_GNU_SOURCE

all: fachoda.sdl fachodasrv

fachoda.sdl: $(OBJET) $(V_SDL) $(MAIN)
	$(CC) $(CFLAGS) -o $@ $(MAIN) $(OBJET) $(V_SDL) -lm -lSDL -lpthread -ljpeg

peindre: $(OBJ2) $(P2)
	$(CC) -o peindre $(OBJ2) $(P2) -L/usr/X11R6/lib -lm -lX11 -lXext -lXxf86dga -O

clean:
	rm -f *.o fachoda.sdl fachodasrv core
