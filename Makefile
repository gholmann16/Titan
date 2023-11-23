trion: commands.o menu.o main.o explorer.o tools.o
	cc main.o -g `pkg-config --libs gtksourceview-4` commands.o menu.o tools.o explorer.o -o Triton
main.o: main.c
	cc main.c -c `pkg-config --cflags gtksourceview-4`
commands.o: commands.c
	cc commands.c -c `pkg-config --cflags gtksourceview-4` -Wno-deprecated-declarations
menu.o: menu.c
	cc menu.c -c `pkg-config --cflags gtksourceview-4`
explorer.o: explorer.c
	cc explorer.c -c `pkg-config --cflags gtksourceview-4`
tools.o: tools.c
	cc tools.c -c `pkg-config --cflags gtksourceview-4`

clean:
	rm *.o

appimage: commands.o menu.o main.o tools.o explorer.o
	cc -O3 release/AppRun.c -o release/AppRun
	cc -O3 main.o `pkg-config --libs gtksourceview-4` commands.o menu.o tools.o explorer.o -o Triton
	strip release/AppRun
	strip Triton
	mkdir -p release/usr/lib
	mkdir -p release/usr/bin
	mv Triton release/usr/bin
	cp assets/Triton.png release
	sed -i -e 's#/usr#././#g' release/usr/bin/Triton
	cp /usr/lib/libgtksourceview-4.so.0 release/usr/lib
	appimagetool release
