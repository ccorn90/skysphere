#################################################################################################################
## trunk/Makefile - Makefile for entire project
## Skysphere Project, ATP 2012, St. Olaf College
## Scroll down to bottom for linking options for specific executables

#################################################################################################################
## Global Compiler Options
CC= g++
CFLAGS= -g -c -Iinclude #-O3
LFLAGS= -L./lib -Wall

all: bin/grab-pano bin/camera-terminal bin/shutdown-cameras #bin/pano-to-ppm

default: all

run: bin/camera-terminal
	./bin/camera-terminal

#run: bin/pano-to-ppm
#	time ./bin/pano-to-ppm -c data/default.panoc

.PHONY : clean
clean:
	rm -f src/*~ include/*~ build/*.o *~
	rm -f 

.PHONY : cleandist
cleandist: clean


################################################################################################################### Library imports
## Eriol ##
ERIOL_LIBS = -leriol -lGL -lglut -lm
lib/liberiol.a:
	make -C ../resources/eriol bin/liberiol.a
	cp ../resources/eriol/bin/liberiol.a lib/

## Visca ##
lib/libvisca.a:
	cd ../resources/libvisca-1.1.0 ; ./configure ; make
	cp ../resources/libvisca-1.1.0/visca/libvisca.a lib/

build/libviscaBRC300.o: src/libviscaBRC300.cpp include/libviscaBRC300.hpp
	$(CC) $(CFLAGS) -Wall -o $@ src/libviscaBRC300.cpp

build/libviscaEVIHD1.o: src/libviscaEVIHD1.cpp include/libviscaEVIHD1.hpp
	$(CC) $(CFLAGS) -Wall -o $@ src/libviscaEVIHD1.cpp

lib/libviscaBRC300.a: build/libviscaBRC300.o
	ar ru $@ build/libviscaBRC300.o

lib/libviscaEVIHD1.a: build/libviscaEVIHD1.o
	ar ru $@ build/libviscaEVIHD1.o

libs: lib/libvisca.a lib/liberiol.a lib/libviscaBRC300.a lib/libviscaEVIHD1.a


#################################################################################################################
## Basic object files
## PanoImage ##
OMPFLAGS= -fopenmp -D__SKYSPHERE_OPEN_MP_FLAG__
PANO_IMAGE_LIBS= $(ERIOL_LIBS) $(OMPFLAGS)
PANO_IMAGE_OBJS= build/PanoImage.o

build/PanoImage.o: include/PanoImage.hpp src/PanoImage.cpp
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ src/PanoImage.cpp

## ImgCap ##
IMAGE_CAPTURE_LIBS= #$(ERIOL_LIBS)
IMAGE_CAPTURE_OBJS= build/ImgCap_V4L2.o  # build/ImgCap_blackmagic.o

build/ImgCap_V4L2.o: include/ImgCap_V4L2.hpp include/ImgCap_base.hpp src/ImgCap_V4L2.cpp
	$(CC) $(CFLAGS) -o $@ src/ImgCap_V4L2.cpp

#build/ImgCap_blackmagic.o: include/ImgCap_blackmagic.hpp include/ImgCap_base.hpp src/ImgCap_blackmagic.cpp
#	$(CC) $(CFLAGS) -o $@ src/ImgCap_blackmagic.cpp

## CameraController ##
CAMERA_CONTROLLER_LIBS= -lvisca -lviscaBRC300 -lviscaEVIHD1
CAMERA_CONTROLLER_OBJS= build/CamCtrl_BRC300.o build/CamCtrl_EVIHD1.o

build/CamCtrl_BRC300.o: include/CamCtrl_base.hpp include/CamCtrl_BRC300.hpp include/libviscaBRC300.hpp include/libvisca.h src/CamCtrl_BRC300.cpp
	$(CC) $(CFLAGS) -o $@ src/CamCtrl_BRC300.cpp

build/CamCtrl_EVIHD1.o: include/CamCtrl_base.hpp include/CamCtrl_EVIHD1.hpp include/libviscaEVIHD1.hpp include/libvisca.h src/CamCtrl_EVIHD1.cpp
	$(CC) $(CFLAGS) -o $@ src/CamCtrl_EVIHD1.cpp

## PanoramicCamera ##
PANORAMIC_CAMERA_LIBS= $(IMAGE_CAPTURE_LIBS) $(CAMERA_CONTROLLER_LIBS) #$(PANO_IMAGE_LIBS)
PANORAMIC_CAMERA_OBJS= build/PanoramicCamera.o $(IMAGE_CAPTURE_OBJS) $(CAMERA_CONTROLLER_OBJS) # $(PANO_IMAGE_OBJS)

build/PanoramicCamera.o: include/PanoramicCamera.hpp src/PanoramicCamera.cpp
	$(CC) $(CFLAGS) -o $@ src/PanoramicCamera.cpp

build/PanoramicCameraDriver.o: src/PanoramicCameraDriver.cpp
	$(CC) $(CFLAGS) -o $@ src/PanoramicCameraDriver.cpp

#################################################################################################################
## Object files for main executables
build/pano-to-ppm.o: main/pano-to-ppm.cpp include/PanoImage.hpp
	$(CC) $(CFLAGS) -o $@ main/pano-to-ppm.cpp

build/grab-pano.o: main/grab-pano.cpp include/PanoramicCamera.hpp $(IMAGE_CAPTURE_OBJS)
	$(CC) $(CFLAGS) -o $@ main/grab-pano.cpp

build/camera-terminal.o: main/camera-terminal.cpp include/PanoramicCamera.hpp $(IMAGE_CAPTURE_OBJS)
	$(CC) $(CFLAGS) -o $@ main/camera-terminal.cpp

build/shutdown-cameras.o: main/shutdown-cameras.cpp include/PanoramicCamera.hpp $(IMAGE_CAPTURE_OBJS)
	$(CC) $(CFLAGS) -o $@ main/shutdown-cameras.cpp

build/repair-byte-order.o: main/repair-byte-order.cpp include/PanoImage.hpp
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ main/repair-byte-order.cpp

build/yuv-to-rgb.o: main/yuv-to-rgb.cpp include/PanoImage.hpp
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ main/yuv-to-rgb.cpp

#################################################################################################################
## Main executables
bin/grab-pano: build/grab-pano.o $(PANORAMIC_CAMERA_OBJS) libs
	$(CC) $(LFLAGS) -o $@ build/grab-pano.o \
		$(PANORAMIC_CAMERA_OBJS) \
		$(PANORAMIC_CAMERA_LIBS)

bin/repair-byte-order: build/repair-byte-order.o $(PANO_IMAGE_OBJS) libs
	$(CC) $(LFLAGS) $(OMPFLAGS) -o $@ build/repair-byte-order.o \
		$(PANO_IMAGE_OBJS) \
		$(PANO_IMAGE_LIBS)

bin/yuv-to-rgb: build/yuv-to-rgb.o $(PANO_IMAGE_OBJS) libs
	$(CC) $(LFLAGS) $(OMPFLAGS) -o $@ build/yuv-to-rgb.o \
		$(PANO_IMAGE_OBJS) \
		$(PANO_IMAGE_LIBS)

bin/shutdown-cameras: build/shutdown-cameras.o $(PANORAMIC_CAMERA_OBJS) libs
	$(CC) $(LFLAGS) -o $@ build/shutdown-cameras.o \
		$(PANORAMIC_CAMERA_OBJS) \
		$(PANORAMIC_CAMERA_LIBS)

bin/camera-terminal: build/camera-terminal.o $(PANORAMIC_CAMERA_OBJS) libs
	$(CC) $(LFLAGS) -o $@ build/camera-terminal.o \
		$(PANORAMIC_CAMERA_OBJS) \
		$(PANORAMIC_CAMERA_LIBS)

bin/pano-to-ppm: build/pano-to-ppm.o $(PANO_IMAGE_OBJS) libs
	$(CC) $(LFLAGS) $(OMPFLAGS) -o $@ build/pano-to-ppm.o \
		$(PANO_IMAGE_OBJS) \
		$(PANO_IMAGE_LIBS)
