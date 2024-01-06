INC_FLAGS := `pkg-config --cflags gtksourceview-4`
CFLAGS := -c -g

ifeq ($(RELEASE),yes)
	CFLAGS := $(CFLAGS) -O3
endif

triton: commands.o menu.o main.o explorer.o tools.o searcher.o gitter.o file.o
	cc `pkg-config --libs gtksourceview-4` main.o commands.o menu.o tools.o explorer.o searcher.o gitter.o file.o -o triton
main.o: main.c
	cc main.c $(CFLAGS) $(INC_FLAGS)
commands.o: commands.c
	cc commands.c $(CFLAGS) $(INC_FLAGS) -Wno-deprecated-declarations
menu.o: menu.c
	cc menu.c $(CFLAGS) $(INC_FLAGS)
explorer.o: explorer.c
	cc explorer.c $(CFLAGS) $(INC_FLAGS)
searcher.o: searcher.c
	cc searcher.c $(CFLAGS) $(INC_FLAGS)
tools.o: tools.c
	cc tools.c $(CFLAGS) $(INC_FLAGS)
gitter.o: gitter.c
	cc gitter.c $(CFLAGS) $(INC_FLAGS)
file.o: file.c
	cc file.c $(CFLAGS)

clean:
	rm *.o

install: triton
	strip triton
	install -d /usr/bin/
	install triton /usr/bin/
	install -d /usr/share/pixmaps/
	install assets/triton.png /usr/share/pixmaps/
	install -d /usr/share/applications/
	install release/triton.desktop /usr/share/applications/

uninstall:
	rm /usr/bin/triton
	rm /usr/share/pixmaps/triton.png
	rm /usr/share/applications/triton.desktop

appimage: triton
	cc -O3 release/AppRun.c -o release/AppRun
	strip release/AppRun
	strip triton
	mkdir -p release/usr/lib
	mkdir -p release/usr/bin
	mv triton release/usr/bin
	cp assets/triton.png release
	sed -i -e 's#/usr#././#g' release/usr/bin/triton
	cp /usr/lib/libgtksourceview-4.so.0 release/usr/lib
	appimagetool release
