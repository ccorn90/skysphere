// PanoramicCameraDriver.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/10/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Driver for PanoramicCamera class.

#include "PanoramicCamera.hpp"

#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;


int main(int argc, char* argv[])
{
  // Set up camera objects
  PanoramicCamera cameraL ("cameraL", "/dev/video0", 2, "/dev/ttyUSB1", 1);
  PanoramicCamera cameraR ("cameraR", "/dev/video1", 2, "/dev/ttyUSB0", 1);
  
  // Initialize cameras
  if(cameraL.init())
    cerr << "Initialized " << cameraL.name() << endl;
  else
    cerr << "Error initializing " << cameraL.name() << endl;

  if(cameraR.init())
    cerr << "Initialized " << cameraR.name() << endl;
  else
  cerr << "Error initializing " << cameraR.name() << endl;
  

  // Check good state
  cerr << "cameraL good? " << cameraL.good() << endl;
  cerr << "cameraR good? " << cameraR.good() << endl;
  
  if(!cameraL.good() && !cameraR.good())
    {
      cerr << "No cameras return good.  Exiting." << endl;
      return 0;
    }
  
  // try to take three panoramic images
  
  string dir1 = "data/panos/1.17/full1x";
  cerr << "Taking panoramic image to directory " << dir1 << endl;
  int r1 = cameraR.grabPano(dir1, -170.0, 170.0, -30.0, 90.0, 0.0, 1, 1, true, "pano");
  cerr << "Return code " << r1 << endl;
  
  string dir2 = "data/panos/1.17/full2x";
  cerr << "Taking panoramic image to directory " << dir2 << endl;
  int r2 = cameraR.grabPano(dir2, -170.0, 170.0, -30.0, 90.0, 0.0, 2, 1, true, "pano");
  cerr << "Return code " << r2 << endl;

  string dir3 = "data/panos/1.17/full4x";
  cerr << "Taking panoramic image to directory " << dir3 << endl;
  int r3 = cameraR.grabPano(dir3, -170.0, 170.0, -30.0, 90.0, 0.0, 4, 1, true, "pano"); 
  cerr << "Return code " << r3 << endl; 
  
  
  
  
  

  
  // Clean up
  cameraL.setZoom(1);
  cameraR.setZoom(1);
  
  if( cameraL.close() )
    cerr << "Closed " << cameraL.name() << endl;
  else
    cerr << "Error closing " << cameraL.name() << endl;
  
  if( cameraR.close() )
    cerr << "Closed " << cameraR.name() << endl;
    else
    cerr << "Error closing " << cameraR.name() << endl;
}
