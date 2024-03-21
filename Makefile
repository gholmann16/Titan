SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c, src/%.o, $(SOURCES))

PSOURCES := $(wildcard po/*.po)
MOBJECTS := $(patsubst po/%.po, po/%.mo, $(PSOURCES))

CFLAGS := `pkg-config --cflags gtksourceview-4`
LDLIBS := `pkg-config --libs gtksourceview-4`

debug: CFLAGS += -g -Og -Wall
debug: triton

release: CFLAGS += -O3
release: triton
	strip $<

triton: $(OBJECTS) $(MOBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDLIBS) $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@
%.mo: %.po
	msgfmt -o $@ $<

update:
	xgettext $(SOURCES) --keyword=_ -o po/triton.pot
	$(foreach po, $(PSOURCES), msgmerge --update $(po) po/triton.pot;)
	rm -f po/*.po~

clean:
	rm -f $(OBJECTS)
	rm -f $(MOBJECTS)
	rm -f triton

install: release
	install -Dm755 triton $(DESTDIR)/usr/bin/triton
	install -Dm644 data/triton.png $(DESTDIR)/usr/share/icons/hicolor/256x256/apps/triton.png
	install -Dm644 data/triton.desktop $(DESTDIR)/usr/share/applications/triton.desktop
	$(foreach object, $(MOBJECTS), install -Dm644 $(object) $(DESTDIR)/usr/share/locale/$(notdir $(basename $(object)))/LC_MESSAGES/triton.mo;)

uninstall:
	rm /usr/bin/triton
	rm /usr/share/icons/hicolor/256x256/apps/triton.png
	rm /usr/share/applications/triton.desktop
	$(foreach object, $(MOBJECTS), rm /usr/share/locale/$(notdir $(basename $(object)))/LC_MESSAGES/triton.mo;)

appdir: $(MOBJECTS)
	mkdir -p appdir
	cp data/triton.desktop appdir/
	ln -fs usr/bin/triton appdir/AppRun
	ln -fs usr/share/icons/hicolor/256x256/apps/triton.png appdir/triton.png

appimage: DESTDIR += appdir
appimage: appdir install
	appimagetool appdir
