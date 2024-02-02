SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

CFLAGS := `pkg-config --cflags gtksourceview-4`
LDLIBS := `pkg-config --libs gtksourceview-4`

debug: CFLAGS += -g -Og
debug: triton

release: CFLAGS += -O3
release: triton
	strip $<

triton: $(OBJECTS)
	$(CC) $(LDLIBS) $(LDFLAGS) $^ -o $@
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

clean:
	rm -f *.o
distclean: clean
	rm -f triton

install: release
	install -d /usr/bin/
	install triton /usr/bin/
	install -d /usr/share/pixmaps/
	install data/triton.png /usr/share/pixmaps/
	install -d /usr/share/applications/
	install data/triton.desktop /usr/share/applications/

uninstall:
	rm /usr/bin/triton
	rm /usr/share/pixmaps/triton.png
	rm /usr/share/applications/triton.desktop

appdir:
	mkdir appdir
	cp data/triton.desktop appdir/
	cp data/triton.png appdir/triton.png

appimage: release appdir
	mv triton appdir/AppRun
	appimagetool appdir
