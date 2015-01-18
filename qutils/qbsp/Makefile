all:	../bin/qbsp

../bin/qbsp: qbsp
	cp qbsp ../bin/qbsp

qbsp: brush.o bspfile2.o csg4.o map.o merge.o nodraw.o outside.o portals.o \
	qbsp.o region.o solidbsp.o surfaces.o tjunc.o writebsp.o \
	../common/mathlib.o ../common/cmdlib.o ../common/qmalloc.o -lm

dep depend .depend:
	$(CC) -I../common -MM *.c ../common/*.c > .depend

include .depend
