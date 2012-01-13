OBJET=renderer.o map.o modele.o radio.o robot.o tableaubord.o txt.o code.o ran0.o control.o mapping.o manuel.o soleil.o carte.o sound.o ravages.o present.o net.o keycodes.o route.o drawroute.o init.o intro.o
MAIN=naw.o
V_SDL=video_sdl.o
OBJ2=codepeindre.o
P2=peindre.o
CFLAGS=-D_GNU_SOURCE -W -Wall -O3 -I/usr/local/include
# i recomand pgcc. gcc will work too, of course.
CC=gcc #/usr/local/pgcc/bin/gcc
CPP=/usr/local/pgcc/bin/g++

all: sdl fachodasrv

code.o: code.as
	nasm -f elf -w+orphan-labels code.as

.c.o:
	$(CC) -c $< -ffast-math $(CFLAGS)

.cc.o:
	$(CPP) -c $< -ffast-math $(CFLAGS)

fachodasrv: fachodasrv.c
	$(CC) $< -o $@

sdl: $(OBJET) $(V_SDL) $(MAIN)
	$(CC) -o fachoda.sdl $(MAIN) $(OBJET) $(V_SDL) -lm -lSDL -lpthread -lesd -ljpeg $(CFLAGS)

codepeindre.o: codepeindre.as
	nasm -f elf -w+orphan-labels codepeindre.as

peindre: $(OBJ2) $(P2)
	$(CC) -o peindre $(OBJ2) $(P2) -L/usr/X11R6/lib -lm -lX11 -lXext -lXxf86dga -O

srv: fachodasrv.o
	$(CC) -o fachodasrv fachodasrv.o -lm -O
	
clean:
	rm -f *.o fachoda.sdl fachodasrv core
