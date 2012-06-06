// CameraTerminal.cpp
// @author Chris Cornelius
// Created 01/12/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Allows keyboard commands to control PanoramicCamera objects

#include "PanoramicCamera.hpp"

#include <iostream>
#include <string>
#include <unistd.h>
#include <sstream>
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
      cerr << "No cameras are functioning.  Exiting." << endl;
      return 0;
    }
 
  cerr << "Setting 0/0/z1..." << endl;
  cameraL.setPanTilt(0.0, 0.0);
  cameraL.setZoom(1);
  cameraR.setPanTilt(0.0, 0.0);
  cameraR.setZoom(1);
  
  cerr << "Ready for commands." << endl;

  char buffer[256];
  int filecount = 0;
  double panL = 120.0, panR = 0.0;
  double tiltL = 0.0, tiltR = 0.0;
  int zoomL = 1, zoomR = 1;
  // Main program loop
  while (1)
    {
      // print prompt
      cerr << "[";
      if(cameraL.good()) cerr << ""; else cerr << "!1";
      if(cameraR.good()) cerr << ""; else cerr << "!2";
      cerr << " CameraTerminal";
      cerr << "] $ ";
      cerr.flush();


      cin.getline(buffer,256);  // get line
      
      stringstream ss (buffer);
      
      string cameraID;
      string command;
      string arg1;
      string arg2;
      
      ss >> command >> cameraID >> arg1 >> arg2;
      
      
      // parse commands
      if(command == "quit" || command == "exit" || command == "q")
	break;
      
      else if(command == "pano")
	{
	  if(cameraID == "")
	    {
	      cerr << "Specify a camera number, silly!" << endl;
	    }
	  else {
	  cerr << "I will take a pano image with camera " << cameraID << endl;
	  string destdir;
	  double t_min = 0.0;
	  double t_max = 0.0;
	  double phi_min = 0.0;
	  double phi_max = 0.0;
	  int zoom = 1;
	  int shots = 1;
	  cerr << "Enter the destination directory: "; cin >> destdir;
	  cerr << "Please enter all angles in degrees from -170.0 to +170.0" << endl;
	  cerr << "Enter thetaMin: ";  cin >> t_min;
	  cerr << "Enter thetaMax: ";  cin >> t_max;
	  cerr << "Enter tilt min value: "; cin >> phi_min;
	  cerr << "Enter tilt max value: "; cin >> phi_max;
	  cerr << "Enter zoom level: "; cin >> zoom;
	  cerr << "Enter how many pictures per position: "; cin >> shots;

	  if(cameraID == "L")
	    {
	      int r1 = cameraL.grabPano(destdir, t_min, t_max, phi_min, phi_max, 0.2, zoom, shots, true, "pano");
	    }
	  else if(cameraID == "R")
	    {
	      int r1 = cameraR.grabPano(destdir, t_min, t_max, phi_min, phi_max, 0.2, zoom, shots, true, "pano");
	    }
	  }
	}
      
      else if(command == "move")
	{
	  if(cameraID == "") {
	    cerr << "Specify a camera, silly!" << endl;
	  }
	  else {
	  cerr << "I will move camera" << cameraID << " to pan " << arg1 << " and tilt " << arg2 << endl;
	  stringstream ss1(arg1);
	  double p = 0.0;
	  ss1 >> p;
	  stringstream ss2(arg2);
	  double t = 0.0;
	  ss2 >> t;
	  
	  if(cameraID == "L")
	    {
	      panL = p;
	      tiltL = t;
	      cameraL.setPanTilt(panL, tiltL);
	    }
	  if(cameraID == "R")
	    {
	      panR = p;
	      tiltR = t;
	      cameraR.setPanTilt(panR, tiltR);
	    }
	  }
	}


      else if(command == "zoom")
	{
	  if(cameraID == "") {
	    cerr << "Specify a camera, silly!" << endl;
	  }
	  else {
	  cerr << "I will zoom camera " << cameraID << " to zoom " << arg1 << endl;
	  if(cameraID == "L")
	    {
	      cameraL.setZoom(atoi(arg1.c_str()));
	      zoomL = atoi(arg1.c_str());
	    }	 
	  if(cameraID == "R")
	    {
	      cameraR.setZoom(atoi(arg1.c_str())); 
	      zoomR = atoi(arg1.c_str());
	    }
	  }
	}
      
  
      else if(command == "click")
	{
	  if(cameraID == "") {
	    cerr << "Specify a camera, silly!" << endl;
	  }
	  else {
	  if(cameraID == "L")
	    {
	      cerr << "I will grab an image from camera" << cameraID << " without changing settings." << endl;
	      sprintf(buffer, "data/1.13/L_%03i.ppm", filecount++);
	      if(0 == cameraL.grabImageToPPM(buffer) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;
	    }	 
	  if(cameraID == "R")
	    {
	      cerr << "I will grab an image from camera" << cameraID << " without changing settings." << endl;
	      sprintf(buffer, "data/1.13/R_%03i.ppm", filecount++);
	      if(0 == cameraR.grabImageToPPM(buffer) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;
	    }
	  }
	}
      
      
      else if(command == "get")
	{
	  if(cameraID == "") {
	    cerr << "Specify a camera, silly!" << endl;
	  }
	  else {
	  cerr << "I will grab an image from camera" << cameraID << " at zoom " << arg1 << endl;
	  stringstream ss1(arg1);
	  int z = atoi(arg1.c_str());

	  if(cameraID == "L")
	    {
	      stringstream ss1(arg1);
	      ss1 >> z;
	      sprintf(buffer, "data/1.13/L_%03i.ppm", filecount++);
	      if(0 == cameraL.grabImageToPPM(buffer, panL, 0, z) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;
	      zoomL = z;
	    }
	  if(cameraID == "R")
	    {
	      sprintf(buffer, "data/1.13/R_%03i.ppm", filecount++);
	      if(0 == cameraR.grabImageToPPM(buffer, panR, 0, z) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;
	      zoomR = z;
	    }
	  }
	}
      
      
      else if (command == "st")
	{
	  if (cameraID == "L")
	    cerr << "CameraL: " << panL << " " << tiltL << " zoom " << zoomL << endl;
	  if (cameraID == "R")
	    cerr << "CameraR: " << panR << " " << tiltR << " zoom " << zoomR << endl;
	}

      else if (command == "help")
	{
	  cerr << endl;
	  cerr << "help - display this help" << endl;
	  cerr << "quit, q, exit - exit the program" << endl;
	  cerr << "move [camera] [pan] [tilt] - moves camera (L/R) to pan and tilt (floating point numbers)." << endl;
	  cerr << "zoom [camera] [zoom] - zooms camera (L/R) to zoom ratio (1-12)" << endl;
	  cerr << "click [camera] - takes an image with camera (L/R) without changing settings." << endl;
	  cerr << "get [camera] [zoom] - takes an image with camera (L/R) with zoom ratio zoom (1-12)" << endl;
	  cerr << "pano [camera] - prompts for remaining info to take a panoramic image with camera (L/R)" << endl;
	  cerr << "" << endl;
	}
      
      else
	{
	  cerr << "unknown command: " << command << ".  Type \"help\" for commands list." << endl;
	}
      
    }      
      
      cameraL.setZoom(1);
      //cameraR.setZoom(1);
  





  // Clean up
      if( cameraL.close() )
	cerr << "Closed " << cameraL.name() << endl;
      else
	cerr << "Error closing " << cameraL.name() << endl;
  
      if( cameraR.close() )
	cerr << "Closed " << cameraR.name() << endl;
      else
	cerr << "Error closing " << cameraR.name() << endl;
}
