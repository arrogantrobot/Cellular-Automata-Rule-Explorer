CC= g++ -o

CCL= g++ -c

PREFIX = /opt/gnome

CFLAGS = `pkg-config --cflags libgnomeui-2.0 gtk+-2.0` -ansi -Wall \
			-DPREFIX=\""$(PREFIX)"\" \
			-DDATADIR=\""$(PREFIX)/share"\" \
			-DSYSCONFDIR=\""$(PREFIX)/etc"\" \
			-DLIBDIR=\""$(PREFIX)/lib"\"
			
LIBS = `pkg-config --libs libgnomeui-2.0 gtk+-2.0`

ca: main.cpp CellularAutomaton.o
	$(CC) ca main.cpp CellularAutomaton.o  $(CFLAGS) $(LIBS)
	
CellularAutomaton.o: CellularAutomaton.cpp CellularAutomaton.h
	$(CCL) CellularAutomaton.cpp

clean:
	/bin/rm -f *~ *.o ca
