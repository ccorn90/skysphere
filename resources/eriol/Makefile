OPTS=-Wall -g
#OPTS=-Wall -g -pg -O
#OPTS=-Wall -O3
CC=g++
ARCH := $(shell uname)
ifeq ($(ARCH), Linux)
 LIBS = -lGL -lglut -lm
else
 MACOSX_DEFINE = -DMACOSX -I/sw/include
 LIBS = -L/System/Library/Frameworks/OpenGL.framework/Libraries -framework GLUT -framework OpenGL -lGL -lm

endif

bin/liberiol.a: eriolObjs.o eriolObjsExtension.o
	ar ruv bin/liberiol.a eriolObjs.o eriolObjsExtension.o


all: bin/liberiol.a bin/eriol

bin/eriol: eriolMain.cpp eriolHeader.h eriolObjs.o
	$(CC) $(OPTS) $(MACOSX_DEFINE) eriolMain.cpp eriolObjs.o $(LIBS) -o $@

eriolObjsExtension.o: eriolObjsExtension.cpp eriolHeader.h
	$(CC) -c eriolObjsExtension.cpp -o $@

clean:
        # NOTE: DO NOT REMOVE eriolObjs.o !!
	rm -f bin/liberiol.a
	rm -f eriolObjsExtension.o
