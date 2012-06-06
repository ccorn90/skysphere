ALLFLAGS = -g -L./lib
SOURCEDIR = src/
BUILDDIR = build/
BLACKMAGICPATH = -I ../resources/blackmagicSDK/Linux/include
BLACKMAGIC_INCLUDE = -I ../resources/blackmagicSDK/Linux/include
BLACKMAGIC_LIBS = -fno-rtti -lm -ldl -lpthread

#########################################################
## For linking with Eirol
ERIOL_LIBS = -leriol -lGL -lglut -lm
#########################################################

default: bin/BlackmagicCaptureTest

all: bin/ImageCaptureDriver

clean:
	rm $(BUILDDIR)*.o bin/rvtx bin/test-rvtx
	rm bin/ImageCaptureDriver


## to import Eriol library
import_liberiol:
	cp ../resources/eriol/bin/liberiol.a lib/


bin/BlackmagicCaptureTest: build/BlackmagicCaptureTest.o build/ImgCap_Blackmagic.o build/DeckLinkAPIDispatch.o
	g++ -o $@ build/BlackmagicCaptureTest.o build/ImgCap_Blackmagic.o build/DeckLinkAPIDispatch.o \
		$(BLACKMAGIC_LIBS) 


build/BlackmagicCaptureTest.o: src/BlackmagicCaptureTest.cpp
	g++ $(BLACKMAGIC_INCLUDE) -o $@ -c src/BlackmagicCaptureTest.cpp

build/ImgCap_Blackmagic.o: src/ImgCap_Blackmagic.cpp
	g++ $(BLACKMAGIC_INCLUDE) -o $@ -c src/ImgCap_Blackmagic.cpp

# a blackmagic library sort of thing... eventually make a .a out of it?
build/DeckLinkAPIDispatch.o: src/DeckLinkAPIDispatch.cpp
	g++ $(BLACKMAGIC_INCLUDE) -o $@ -c src/DeckLinkAPIDispatch.cpp


bin/BlackmagicTest: $(BUILDDIR)main.o
	g++ $(ALLFLAGS) $(BLACKMAGICPATH) -o bin/BlackmagicTest $(BUILDDIR)main.o -fno-rtti -lm -ldl -lpthread


bin/ImageCaptureDriver: $(BUILDDIR)ImageCaptureDriver.o $(BUILDDIR)ImageCapture.o
	g++ $(ALLFLAGS) -o bin/ImageCaptureDriver \
		$(BUILDDIR)ImageCaptureDriver.o \
		$(BUILDDIR)ImageCapture.o \
		$(ERIOL_LIBS)

$(BUILDDIR)main.o: $(SOURCEDIR)main.cpp $(SOURCEDIR)main.h
	pwd; g++ $(ALLFLAGS) $(BLACKMAGICPATH) -c $(SOURCEDIR)main.cpp -o $(BUILDDIR)main.o

$(BUILDDIR)ImageCapture.o: $(SOURCEDIR)ImageCapture.cpp $(SOURCEDIR)ImageCapture.h $(SOURCEDIR)RawImage.h
	g++ $(ALLFLAGS) -c $(SOURCEDIR)ImageCapture.cpp -o $(BUILDDIR)ImageCapture.o

$(BUILDDIR)ImageCaptureDriver.o: src/ImageCaptureDriver.cpp
	g++ $(ALLFLAGS) -c $(SOURCEDIR)ImageCaptureDriver.cpp -o $(BUILDDIR)ImageCaptureDriver.o

########################################################
## Old stuff -- wiebe's code for rvtx.
## For new image capture code as of 1/2012, see above

bin/rvtx: $(BUILDDIR)rvtx.o $(BUILDDIR)GenericDatagram.o $(BUILDDIR)HostGroup.o $(BUILDDIR)Logger.o $(SOURCEDIR)rvc_types.h $(SOURCEDIR)config.h
	g++ $(ALLFLAGS) -o bin/rvtx $(BUILDDIR)rvtx.o $(BUILDDIR)GenericDatagram.o $(BUILDDIR)HostGroup.o $(BUILDDIR)Logger.o

bin/test-rvtx: $(BUILDDIR)test-rvtx.o $(BUILDDIR)GenericDatagram.o $(BUILDDIR)HostGroup.o $(BUILDDIR)Logger.o $(SOURCEDIR)rvc_types.h $(SOURCEDIR)config.h
	g++ $(ALLFLAGS) -o bin/test-rvtx $(BUILDDIR)test-rvtx.o $(BUILDDIR)GenericDatagram.o $(BUILDDIR)HostGroup.o $(BUILDDIR)Logger.o

$(BUILDDIR)GenericDatagram.o: $(SOURCEDIR)GenericDatagram.cpp $(SOURCEDIR)GenericDatagram.h $(SOURCEDIR)HostGroup.h $(SOURCEDIR)Logger.h
	g++ $(ALLFLAGS) -c $(SOURCEDIR)GenericDatagram.cpp -o $(BUILDDIR)GenericDatagram.o

$(BUILDDIR)HostGroup.o: $(SOURCEDIR)HostGroup.cpp $(SOURCEDIR)GenericDatagram.h $(SOURCEDIR)HostGroup.h $(SOURCEDIR)Logger.h
	g++ $(ALLFLAGS) -c $(SOURCEDIR)HostGroup.cpp -o $(BUILDDIR)HostGroup.o

$(BUILDDIR)Logger.o: $(SOURCEDIR)Logger.cpp $(SOURCEDIR)GenericDatagram.h $(SOURCEDIR)HostGroup.h $(SOURCEDIR)Logger.h
	g++ $(ALLFLAGS) -c $(SOURCEDIR)Logger.cpp -o $(BUILDDIR)Logger.o

$(BUILDDIR)test-rvtx.o: $(SOURCEDIR)test-rvtx.cpp
	g++ $(ALLFLAGS) -c $(SOURCEDIR)test-rvtx.cpp -o $(BUILDDIR)test-rvtx.o

$(BUILDDIR)rvtx.o: $(SOURCEDIR)rvtx.cpp
	g++ $(ALLFLAGS) -c $(SOURCEDIR)rvtx.cpp -o $(BUILDDIR)rvtx.o
