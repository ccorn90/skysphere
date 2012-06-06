// This file contains methods and variables that are defined as "extern" in eriolObjs.o
//   in order to get complete linking when using the liberiol.a library generated from
//   the makefile in this directory.
//   Modified from Olaf Hall-Holt's eriolMain.cpp 1/9/2012
//   Chris Cornelius, ATP 2012

#include"eriolHeader.h"

void assignImgBBox();
void resetZoom(bool img2);
void removeCurrImage(bool img2);
void init_gl_window();

int mainWindowID = -1, secondWindowID = -1;
bool NO_DISPLAY=false;
extern ImageUI::UIMode theUIMode;
extern bool realTimeInfo;
bool leftMouseButtonIsDown = false;
bool middleMouseButtonIsDown = false;
bool rightMouseButtonIsDown = false;
bool shiftPressed = false;
bool ctrlPressed = false;
PixelLoc currXY(0,0);
Coord startImagePt;
Coord startFocalLength;

void commonKeyboard( unsigned char c, int x, int y, bool inSecondWindow )
{ }

void mouse(int button, int state, int x, int y, bool inSecondWindow)
{ }
