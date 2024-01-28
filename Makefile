INC_FLAGS := `pkg-config --cflags gtksourceview-4`
CFLAGS := -c -g

triton: commands.o menu.o main.o explorer.o
	cc `pkg-config --libs gtksourceview-4` main.o commands.o menu.o explorer.o -o triton
main.o: main.c
	cc main.c $(CFLAGS) $(INC_FLAGS)
commands.o: commands.c
	cc commands.c $(CFLAGS) $(INC_FLAGS) -Wno-deprecated-declarations
menu.o: menu.c
	cc menu.c $(CFLAGS) $(INC_FLAGS)
explorer.o: explorer.c
	cc explorer.c $(CFLAGS) $(INC_FLAGS)

clean:
	rm *.o

release: CFLAGS += -O3
release: clean triton
	strip triton

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
