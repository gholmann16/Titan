SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c, src/%.o, $(SOURCES))

PSOURCES := $(wildcard po/*.po)
MOBJECTS := $(patsubst po/%.po, po/%.mo, $(PSOURCES))

CFLAGS := `pkg-config --cflags gtksourceview-4 vte-2.91`
LDLIBS := `pkg-config --libs gtksourceview-4 vte-2.91`

debug: CFLAGS += -g -Og -Wall
debug: titan

release: CFLAGS += -O3
release: titan
	strip $<

titan: $(OBJECTS) $(MOBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDLIBS) $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@
%.mo: %.po
	msgfmt -o $@ $<

update:
	xgettext $(SOURCES) --keyword=_ -o po/titan.pot
	$(foreach po, $(PSOURCES), msgmerge --update $(po) po/titan.pot;)
	rm -f po/*.po~

clean:
	rm -f $(OBJECTS)
	rm -f $(MOBJECTS)
	rm -f titan

install: release
	install -Dm755 titan $(DESTDIR)/usr/bin/titan
	install -Dm644 data/titan.png $(DESTDIR)/usr/share/icons/hicolor/256x256/apps/titan.png
	install -Dm644 data/titan.desktop $(DESTDIR)/usr/share/applications/titan.desktop
	$(foreach object, $(MOBJECTS), install -Dm644 $(object) $(DESTDIR)/usr/share/locale/$(notdir $(basename $(object)))/LC_MESSAGES/titan.mo;)

uninstall:
	rm /usr/bin/titan
	rm /usr/share/icons/hicolor/256x256/apps/titan.png
	rm /usr/share/applications/titan.desktop
	$(foreach object, $(MOBJECTS), rm /usr/share/locale/$(notdir $(basename $(object)))/LC_MESSAGES/titan.mo;)

appdir: $(MOBJECTS)
	mkdir -p appdir
	cp data/titan.desktop appdir/
	ln -fs usr/bin/titan appdir/AppRun
	ln -fs usr/share/icons/hicolor/256x256/apps/titan.png appdir/titan.png

appimage: DESTDIR += appdir
appimage: appdir install
	appimagetool appdir
