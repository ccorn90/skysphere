// CameraControllerDriver.cpp
// @author Chris Cornelius
// Created 01/05/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Driver to test CameraController class

#include "CameraController.hpp"
#include <iostream>
#include <cstring>
using namespace std;

static string DEFAULT_SERIAL_DEVICE ("/dev/ttyUSB0");

int main(int argc, char* argv[])
{
  // config vars
  string devname;
  
  // set up device name to use -- default to DEFAULT_VIDEO_DEVICE
  if(argc < 1)
    {
      devname = argv[1];
    }
  else
    {
      devname = DEFAULT_SERIAL_DEVICE;
    }


  // test the degrees_to_uint method
  double deg = -100.0;
  fprintf(stderr, "%lf into uints = %x\n", deg, libviscaBRC300::degrees_to_uint(deg));


  // the CC object we'll be using
  CameraController c (devname, 1);

  cerr << "Created CameraController object." << endl;
  
  // open connection
  if( c.openSerial() )
    cerr << "Opened serial connection on " << c.serialDevice() << " with camera ID of " << c.viscaID() << "." << endl;
  else
    cerr << "Error opening serial connection." << endl;

  
  // initialize camera
  if( c.initializeCamera() )
    cerr << "Initialized camera." << endl;
  else
    cerr << "Error initializing camera." << endl;

  // have camera seek a position
  if ( c.setPanTilt(-120.0,0.0) )
    cerr << "Moved camera to new position." << endl;
  else
    cerr << "Error moving camera." << endl;
  
  
  // have camera move to a certain zoom
  if( c.setZoom(12) )
    cerr << "Zoomed camera to 12x." << endl;
  else
    cerr << "Error zooming camera!" << endl;
  if( c.setZoom(1) )
    cerr << "Zoomed camera to 1x." << endl;
  else
    cerr << "Error zooming camera!" << endl;
  
  // turn off the camera, for fun
  if( c.setPowerOff() )
    cerr << "Powered camera off." << endl;
  else
    cerr << "Error powering camera!" << endl;

  // turn on the camera, again
  /*
  if( c.setPowerOff() )
    cerr << "Powered camera on." << endl;
  else
    cerr << "Error powering camera!" << endl;
  */

  // close connection
  if( c.closeSerial() )
    cerr << "Closed serial connection." << endl;
  else
    cerr << "Error closing serial connection." << endl;
  
  return 0;
}
