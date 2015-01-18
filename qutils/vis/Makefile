all:	../bin/vis

../bin/vis: vis
	cp vis ../bin/vis

vis: flow.o soundpvs.o vis.o \
	../common/mathlib.o ../common/qmalloc.o ../common/cmdlib.o \
	../common/work.o ../common/bspfile.o -lm

dep depend .depend:
	$(CC) -I../common -MM *.c ../common/*.c > .depend

include .depend
