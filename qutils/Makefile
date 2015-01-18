.EXPORT_ALL_VARIABLES:

CC = gcc
CFLAGS = -Wall -O2 -fomit-frame-pointer -I../common

all: bin/qbsp bin/light bin/vis

bin/qbsp::
	make -C qbsp 

bin/light::
	make -C light 

bin/vis::
	make -C vis 

clean:
	rm */*.o
	rm {bin,qbsp}/qbsp {bin,light}/light {bin,vis}/vis
