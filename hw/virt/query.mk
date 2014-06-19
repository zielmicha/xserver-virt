all: query libvirtdisplay.so

libvirtdisplay.so: query.c query.h
	gcc query.c -shared -o libvirtdisplay.so -Wall -lX11 -I/usr/include/X11 -fPIC

query: query.c query.h
	gcc query.c -DVIRTDISPLAY_WITH_MAIN -o query -Wall -lX11 -I/usr/include/X11
