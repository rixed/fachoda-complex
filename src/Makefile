CFLAGS   := $(CFLAGS) -std=c99 -W -Wall -D_GNU_SOURCE ${shell sdl-config --cflags}
LDFLAGS  := $(LDFLAGS) -lm -ljpeg -lopenal ${shell sdl-config --libs}

SRCS = \
	renderer.c map.c modele.c radio.c robot.c \
	tableaubord.c txt.c control.c mapping.c \
	manuel.c soleil.c carte.c sound.c ravages.c \
	present.c net.c keycodes.c route.c drawroute.c \
	init.c intro.c naw.c video_sdl.c world.c

OBJS = ${subst .c,.o,$(SRCS)}

fachoda: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

.PHONY: clean distclean cscope clear

clean:
	rm -f *.o *.dep core core.*

distclean: clean
	rm -f fachoda fachoda.sdl fachodasrv cscope.out

cscope:
	cscope -Rb $(CPPFLAGS)

clear:
	find . -type f -\( -name '*.c' -o -name '*.h' -\) | xargs sed -i -e 's/[ \t]\+$$//'

.depend:
	rm -f .depend ;
	for f in $(SRCS); do \
		$(CC) -MM -MF $$f.dep $$f ; \
		cat $$f.dep >> .depend ; \
	done

include .depend
