##############################################################
## Global Compiler Options
CC= g++
CFLAGS= -Iinclude -c
LFLAGS= -Wall -L./lib

all: build/CameraController.o bin/CameraControllerDriver bin/testvisca bin/testvisca_chris lib/libviscaBRC300.a

run: bin/CameraControllerDriver
	./bin/CameraControllerDriver

.PHONY : clean
clean:
	rm -f src/*~ include/*~
	rm -f build/*.o
	rm -f *~

.PHONY : cleandist
cleandist: clean
	rm -f bin/testvisca

##############################################################
## For CameraController class
CAMERA_CONTROLLER_LIBS = -lvisca -lviscaBRC300

build/CameraController.o: include/CameraController.hpp include/libvisca.h src/CameraController.cpp
	$(CC) $(CFLAGS) -o build/CameraController.o src/CameraController.cpp

build/CameraControllerDriver.o: src/CameraControllerDriver.cpp include/CameraController.hpp
	$(CC) $(CFLAGS) -o build/CameraControllerDriver.o src/CameraControllerDriver.cpp

build/libviscaBRC300.o: src/libviscaBRC300.cpp include/libviscaBRC300.hpp
	$(CC) $(CFLAGS) -o $@ src/libviscaBRC300.cpp

bin/CameraControllerDriver: build/CameraControllerDriver.o build/CameraController.o build/libviscaBRC300.o
	$(CC) $(LFLAGS) \
		-o bin/CameraControllerDriver \
		build/CameraControllerDriver.o \
		build/CameraController.o \
		$(CAMERA_CONTROLLER_LIBS)

lib/libviscaBRC300.a: build/libviscaBRC300.o
	ar ru $@ build/libviscaBRC300.o

##############################################################
## libvisca test
VISCA_LIBS = -lvisca

# one of the examples from libvisca
build/testvisca.o: src/testvisca.c
	$(CC) $(CFLAGS) -o build/testvisca.o src/testvisca.c

build/testvisca_chris.o: src/testvisca_chris.c
	$(CC) $(CFLAGS) -o build/testvisca_chris.o src/testvisca_chris.c

bin/testvisca: build/testvisca.o 
	$(CC) $(LFLAGS) -o bin/testvisca build/testvisca.o $(VISCA_LIBS)

bin/testvisca_chris: build/testvisca_chris.o 
	$(CC) $(LFLAGS) -o bin/testvisca_chris build/testvisca_chris.o $(VISCA_LIBS)


