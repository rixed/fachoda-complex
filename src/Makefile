CFLAGS   := $(CFLAGS) -std=c99 -W -Wall ${shell sdl-config --cflags}
CPPFLAGS := $(CPPFLAGS) -D_GNU_SOURCE
LDFLAGS  := $(LDFLAGS) -lm -ljpeg ${shell sdl-config --libs}

SRCS = \
	renderer.c map.c modele.c radio.c robot.c \
	tableaubord.c txt.c ran0.c control.c mapping.c \
	manuel.c soleil.c carte.c sound.c ravages.c \
	present.c net.c keycodes.c route.c drawroute.c \
	init.c intro.c naw.c video_sdl.c

OBJS = ${subst .c,.o,$(SRCS)}

fachoda: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

.PHONY: clean distclean

clean:
	rm -f *.o *.dep core core.*

distclean: clean
	rm -f fachoda fachoda.sdl fachodasrv

.depend: $(SRCS)
	rm -f .depend ;
	for f in $(SRCS); do \
		$(CC) -MM -MF $$f.dep $$f ; \
		cat $$f.dep >> .depend ; \
	done

include .depend
