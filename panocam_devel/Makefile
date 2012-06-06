###############################################################
## Global Compiler Options
CC= g++
CFLAGS= -Iinclude -c -O3
LFLAGS= -Wall -Llib -O3

###############################################################
## General complilation targets
default: clearscreen bin/PanoramicCameraDriver bin/CameraTerminal bin/PanoImageDriver

run: bin/CameraTerminal bin/PanoramicCameraDriver bin/PanoImageDriver
#	@./bin/PanoramicCameraDriver
	@make run_panoImage

.PHONY : clean
clean:
	rm -f src/*~ include/*~
	rm -f build/*.o
	rm -f *~
	rm -f bin/PanoImageDriver

.PHONY : cleandist
cleandist: clean

clearscreen:
	clear

import_all: import_liberiol


##############################################################
## For linking with Eirol
ERIOL_LIBS = -leriol -lGL -lglut -lm
##############################################################

## to import Eriol library
import_liberiol:
	cp ../resources/eriol/bin/liberiol.a lib/



##############################################################
## PanoImage
## NOTE 1/12/12: Will not link on specs because of old version of Fedora and OpenGL libs
#OMPFLAGS= -fopenmp -D__SKYSPHERE_OPEN_MP_FLAG__ ## For OpenMP parallelism -- be sure to protect OpenMP features that you use in your code with an #ifdef statement!
PANO_IMAGE_LIBS= $(ERIOL_LIBS) $(OMPFLAGS)
PANO_IMAGE_OBJS= build/PanoImage.o

build/PanoImage.o: include/PanoImage.hpp src/PanoImage.cpp
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ src/PanoImage.cpp

build/PanoImageDriver.o: include/PanoImage.hpp src/PanoImageDriver.cpp
	$(CC) $(CFLAGS) -o $@ src/PanoImageDriver.cpp

bin/PanoImageDriver: build/PanoImageDriver.o $(PANO_IMAGE_OBJS)
	$(CC) $(LFLAGS) -o $@ \
		build/PanoImageDriver.o \
		$(PANO_IMAGE_OBJS) \
		$(PANO_IMAGE_LIBS)

run_panoImage:
	@time ./bin/PanoImageDriver
	@echo "make: Displaying image data/out.ppm"
	@eog data/out.ppm # display output image



##############################################################
## For ImageCapture class
IMAGE_CAPTURE_LIBS= #$(ERIOL_LIBS)
IMAGE_CAPTURE_OBJS= build/ImageCapture.o

build/ImageCapture.o: src/ImageCapture.cpp include/ImageCapture.h
	$(CC) $(CFLAGS) -o $@ src/ImageCapture.cpp


##############################################################
## For CameraController class
CAMERA_CONTROLLER_LIBS= -lvisca -lviscaBRC300
CAMERA_CONTROLLER_OBJS= build/CameraController.o

build/CameraController.o: include/CameraController.hpp include/libvisca.h src/CameraController.cpp
	$(CC) $(CFLAGS) -o build/CameraController.o src/CameraController.cpp

build/libviscaBRC300.o: src/libviscaBRC300.cpp include/libviscaBRC300.hpp
	$(CC) $(CFLAGS) -o $@ src/libviscaBRC300.cpp

lib/libviscaBRC300.a: build/libviscaBRC300.o
	ar ru $@ build/libviscaBRC300.o


##############################################
## PanoramicCamera 
PANORAMIC_CAMERA_LIBS= $(IMAGE_CAPTURE_LIBS) $(CAMERA_CONTROLLER_LIBS) # $(PANO_IMAGE_LIBS)
PANORAMIC_CAMERA_OBJS= $(IMAGE_CAPTURE_OBJS) $(CAMERA_CONTROLLER_OBJS) build/PanoramicCamera.o

build/PanoramicCamera.o: include/PanoramicCamera.hpp src/PanoramicCamera.cpp
	$(CC) $(CFLAGS) -o $@ src/PanoramicCamera.cpp

build/PanoramicCameraDriver.o: src/PanoramicCameraDriver.cpp
	$(CC) $(CFLAGS) -o $@ src/PanoramicCameraDriver.cpp

bin/PanoramicCameraDriver: build/PanoramicCameraDriver.o $(PANORAMIC_CAMERA_OBJS) lib/libviscaBRC300.a
	$(CC) $(LFLAGS) -o $@ \
		build/PanoramicCameraDriver.o \
		$(PANORAMIC_CAMERA_OBJS) \
		$(PANORAMIC_CAMERA_LIBS)

build/CameraTerminal.o: src/CameraTerminal.cpp
	$(CC) $(CFLAGS) -o $@ src/CameraTerminal.cpp

bin/CameraTerminal: build/CameraTerminal.o $(PANORAMIC_CAMERA_OBJS) lib/libviscaBRC300.a
	$(CC) $(LFLAGS) -o $@ \
		build/CameraTerminal.o \
		$(PANORAMIC_CAMERA_OBJS) \
		$(PANORAMIC_CAMERA_LIBS)
