all:	../bin/light

../bin/light: light
	cp light ../bin/light

light: entities.o light.o ltface.o threads.o trace.o \
	../common/cmdlib.o ../common/bspfile.o ../common/work.o \
	../common/mathlib.o ../common/qmalloc.o -lm

dep depend .depend:
	$(CC) -I../common -MM *.c ../common/*.c > .depend

include .depend
