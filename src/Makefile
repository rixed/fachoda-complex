PREFIX   := /usr/local
BINDIR   := $(PREFIX)/games
DATADIR  := $(PREFIX)/lib/games/fachoda
DOCDIR   := $(PREFIX)/share/doc/fachoda
CFLAGS   := $(CFLAGS) -std=c99 -W -Wall -D_GNU_SOURCE ${shell sdl-config --cflags} -DDATADIR='"$(DATADIR)"'
LDFLAGS  := $(LDFLAGS) -lm -ljpeg -lopenal ${shell sdl-config --libs}

SRCS = \
	renderer.c heightfield.c model.c radio.c robot.c \
	instruments.c txt.c physics.c mapping.c \
	control.c sun.c map.c sound.c ravages.c \
	present.c keycodes.c roads.c \
	init.c intro.c main.c video_sdl.c world.c \
	gtime.c file.c

OBJS = ${subst .c,.o,$(SRCS)}

all: fachoda

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
	install ../COPYING ../GPL.txt ../README.md $(DESTDIR)$(DOCDIR)
	@for d in $(SUBDATADIRS); do \
		install -d "$(DESTDIR)$(DATADIR)/$$d" ;\
		for f in $$d/*; do \
			if ! echo "$$f" | grep .dxf >/dev/null; then \
				echo "Installing $$f" ;\
				install "$$f" "$(DESTDIR)$(DATADIR)/$$d" ;\
			fi \
		done \
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
