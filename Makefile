netsim1: standalone.h standalone.o
	gcc -O -o netsim1 netsim1.c standalone.o -L/usr/lib -lm

standalone.o: standalone.c standalone.h
	gcc -O -c standalone.c

clean:
	/bin/rm standalone.o netsim1
