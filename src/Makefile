PREFIX   := /usr/local
BINDIR   := $(PREFIX)/games
DATADIR  := $(PREFIX)/lib/games/fachoda
DOCDIR   := $(PREFIX)/share/doc/fachoda
CFLAGS   := $(CFLAGS) -std=c99 -W -Wall -D_GNU_SOURCE ${shell sdl-config --cflags} -DDATADIR='"$(DATADIR)"'
LDFLAGS  := $(LDFLAGS) -lm -ljpeg -lopenal ${shell sdl-config --libs}

SRCS = \
	renderer.c map.c modele.c radio.c robot.c \
	tableaubord.c txt.c control.c mapping.c \
	manuel.c soleil.c carte.c sound.c ravages.c \
	present.c keycodes.c route.c drawroute.c \
	init.c intro.c naw.c video_sdl.c world.c \
	gtime.c file.c

OBJS = ${subst .c,.o,$(SRCS)}

fachoda: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

.PHONY: clean distclean cscope clear install

SUBDATADIRS := \
	snd snd2 \
	arbre2 base1 base1light bovide chalet corsair corsairlight \
	d501 d501light eglise egliselight ferme maison \
	moshito moshitolight moulin reverbere \
	snophill snoplight sol spitflame spitflamelight \
	tank tanklight toyvion toyvionlight tracteur usine \
	van vehic vion1 vion2 zeppelin

COMPDATA = $(wildcard *.jpg) $(wildcard *.tga)
install: fachoda $(COMPDATA)
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(DATADIR) $(DESTDIR)$(DOCDIR)
	install --strip fachoda $(DESTDIR)$(BINDIR)
	install $(COMPDATA) $(DESTDIR)$(DATADIR)
	install ../COPYING ../GPL.txt ../README $(DESTDIR)$(DOCDIR)
	for d in $(SUBDATADIRS); do \
		install -d $(DESTDIR)$(DATADIR)/$$d ;\
		install $$d/* $(DESTDIR)$(DATADIR)/$$d ;\
	done

clean:
	rm -f *.o *.dep .depend core core.*

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
