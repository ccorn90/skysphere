// camera-terminal.cpp
// @author Chris Cornelius
// Created 01/12/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Allows keyboard commands to control PanoramicCamera objects

#include "PanoramicCamera.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <time.h>
using namespace std;

// global methods
int readConfigFile(string filename);
int processArgs(int, char**);
int setUpCamera(PanoramicCamera* c);
PanoramicCamera* findCamera(string cameraname);

// defaults for the program
const static string DEFAULT_CONFIG_FILE = "./data/ctconfig";
const static string DEFAULT_SAVEPATH = "./data/";

// Globals for the program
vector<PanoramicCamera*> cameras;
string configFile = DEFAULT_CONFIG_FILE;
string savePath = DEFAULT_SAVEPATH;

int main(int argc, char* argv[])
{
  // Argument processing
  int a = processArgs(argc, argv);
  if( a != 0 )
    {
      cerr << "Error " << a << " while parsing arguments." << endl;
      cerr << "Try running with --help to start." << endl;
      return -1;
    }

  if(0 == readConfigFile(configFile)) {
    cerr << "Read config file " << configFile << endl;
  }
  else {
    cerr << "Unable to read config file " << configFile << endl;
    cerr << "Try running with [-c configfile] and specifying the settings you want." << endl;
    return 0;
  }
  
  // set up vars we know we'll need
  size_t numCameras = cameras.size();

  // display stats for all cameras  
  cerr << "Initializing " << numCameras << " cameras:" << endl;
  vector<PanoramicCamera*>::iterator it = cameras.begin();
  while(it != cameras.end())
    {
      cerr << "Camera " << *(*it) << endl << (*it)->name() << " ...";
      int r = setUpCamera(*it);
      if(r == 0) {
	cerr << " OK" << endl;
	it++;
      }
      else {
	// if camera didn't initialize, remove it from the array
	cerr << " ERROR " << r << " : camera ignored" << endl;
	cameras.erase(it);
	numCameras--;
      }
    }
  
  if(numCameras < 1)
    {
      cerr << "camera-terminal: No cameras are functioning.  Exiting." << endl;
      return 0;
    }
  
  cerr << "Setting 0/0/z1..." << endl;
  it = cameras.begin();
  while(it != cameras.end())
    {
      (*it)->setPanTilt(0.0, 0.0);
      (*it)->setZoom(1);
      it++;
    }

  cerr << "Ready for commands." << endl;

  // main loop globals
  char buffer[256];
  int filecount = 0;
  PanoramicCamera* cam = NULL;

  // Main program loop
  while (1)
    {
      // print prompt
      cerr << "[";
      cerr << "CameraTerminal";
      cerr << "] $ ";
      cerr.flush();

      cin.getline(buffer,256);  // get line
      
      stringstream ss (buffer);
      
      string cameraID;
      string command;
      string arg1;
      string arg2;
      string arg3;
      
      ss >> command >> cameraID >> arg1 >> arg2 >> arg3;
      
      
      // parse commands
      if(command == "quit" || command == "exit" || command == "q")
	break;
      
      else if(command == "pano")
	{
	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }


	  string destdir;
	  double t_min = 0.0;
	  double t_max = 0.0;
	  double phi_min = 0.0;
	  double phi_max = 0.0;
	  int zoom = 1;
	  int shots = 1;
	  double olap = 0.0;
	  cerr << "Enter the destination directory: "; cin >> destdir;
	  cerr << "Please enter all angles in degrees from -170.0 to +170.0" << endl;
	  cerr << "Enter thetaMin: ";  cin >> t_min;
	  cerr << "Enter thetaMax: ";  cin >> t_max;
	  cerr << "Enter tilt min value: "; cin >> phi_min;
	  cerr << "Enter tilt max value: "; cin >> phi_max;
	  cerr << "Enter zoom level: "; cin >> zoom;
	  cerr << "Enter how many pictures per position: "; cin >> shots;
	  cerr << "Enter overlap factor (0.0 - 1.0): "; cin >> olap;
       
	  int r = cam->grabPano(destdir, t_min, t_max, phi_min, phi_max, olap, zoom, shots, true, "pano");
	  if(r != 0) cerr << "an error occurred while capturing a panorama!" << endl;

	  
	}
      
      else if(command == "move")
	{
	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }

	  stringstream ss1(arg1);
	  double p = 0.0;
	  ss1 >> p;
	  stringstream ss2(arg2);
	  double t = 0.0;
	  ss2 >> t;

	  cam->setPanTilt(p, t);  // TODO: remove tilt fixation!!!!!

	  
	}


      else if(command == "zoom")
	{
       	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }

	  if( cam->setZoom(atoi(arg1.c_str())) )
	    cerr << "Set " << cam->name() << " to zoom " << arg1 << endl;
	  else
	    cerr << "Error setting zoom." << endl;
	}


      else if(command == "grab")
	{
	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }
	  
	  stringstream ss1(arg1);
	  int numPics = 12;
	  ss1 >> numPics;

	  for(int i = 0; i < numPics; i++ ) {
	      cerr << "Grabbing image from " << cam->name() << endl;
	      
	      // get the time from the system
	      time_t t;  struct tm* time_p;
	      time(&t);  time_p = localtime(&t);
	      char timebuffer[64];
	      strftime(timebuffer,64,"%d_%m_%y_%H%M",time_p);  // make a formatted string from the time
	      
	      
	      sprintf(buffer, "%s/%s_%03i_%s.ppm", savePath.c_str(), cam->shortname().c_str(),filecount++,timebuffer);
	      if(0 == cam->grabImageToPPM(buffer) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;
	    }
		
	}


    else if(command == "grab2")
	{
	  cam = findCamera("L");
	  PanoramicCamera* cam2 = findCamera("R");
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }
	  
	  stringstream ss1(arg1);
	  int numPics = 1;
	  ss1 >> numPics;

	  for(int i = 0; i < numPics; i++ ) {
	      cerr << "Grabbing image from " << cam->name() << endl;
	      
	      // get the time from the system
	      time_t t;  struct tm* time_p;
	      time(&t);  time_p = localtime(&t);
	      char timebuffer[64];
	      strftime(timebuffer,64,"%d_%m_%y_%H%M",time_p);  // make a formatted string from the time
	      
	      
	      sprintf(buffer, "%s/%s_%03i_%s.ppm", savePath.c_str(), cam->shortname().c_str(),filecount++,timebuffer);
	      if(0 == cam->grabImageToPPM(buffer) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;

	      cerr << "Grabbing image from " << cam2->name() << endl;
	      
	      // get the time from the system     
	      sprintf(buffer, "%s/%s_%03i_%s.ppm", savePath.c_str(), cam2->shortname().c_str(),filecount++,timebuffer);
	      if(0 == cam2->grabImageToPPM(buffer) )
		cerr << "Grabbed image " << buffer << endl;
	      else
		cerr << "Error grabbing image." << endl;
	    }
		
	}
      else if (command == "info")
	{
	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }

	  cerr << "Camera Info:" << endl << *cam << endl;
	}  
      
      else if (command == "st")
	{
	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }
	  
	  cerr << "This command is a TODO" << endl;
	  
	}

      else if (command == "help")
	{
	  cerr << endl;
	  cerr << "Note: you can always enter either a camera's long name or short name." << endl;
	  cerr << "help - display this help" << endl;
	  cerr << "quit, q, exit - exit the program" << endl;
	  cerr << "st [camera] - prints status for the given camera." << endl;
	  cerr << "info [camera] - prints configuration info for the given camera." << endl;
	  cerr << "move [camera] [pan] [tilt] - moves camera to pan and tilt (floating point numbers)." << endl;
	  //	  cerr << "nudge [camera] [pan] [tilt] - moves camera a relative amount" << endl;  // TODO: implement
	  cerr << "zoom [camera] [zoom] - zooms camera to zoom ratio (1-12)" << endl;
	  cerr << "grab [camera] [numimages] - takes numimages number of image with camera to the filepath configured for the program." << endl;
	  cerr << "setpath [path] - sets the desination directory for grabbed images" << endl;
	  cerr << "pano [camera] - prompts for remaining info to take a panoramic image with camera" << endl;
	  cerr << "exposure [camera] [gain] [shutter] [iris] -- manually sets camera exposure controls... using hex numbers for now" << endl;
	  cerr << "on - powers on all cameras" << endl;
	  cerr << "off - powers off all cameras" << endl;
	  cerr << "" << endl;
	}
      
      else if(command == "exposure" || command == "exp" )
	{
	  cam = findCamera(cameraID);
	  if(cam == NULL) {
	    cerr << "Cannot find camera " << cameraID << endl;
	    continue;
	  }
	  
	  stringstream ss1(arg1);
	  int g = 0;
	  ss1 >> g;
	  stringstream ss2(arg2);
	  int sh = 0;
	  ss2 >> sh;
	  stringstream ss3(arg3);
	  double ir = 1.8;
	  ss3 >> ir;
	  
	  if(arg1 == "auto")
	    {
	      cam->setExposureAuto(false);
	      cerr << "Set " << cam->name() << " to auto exposure." << endl;
	    }
	  else {
	    cerr << "Setting values on " << cam->name() << "...";
	    cam->setExposureAuto(false);
	    cam->setGain((char) g);
	    cam->setShutter((char) sh);
	    cam->setIris(ir);
	    cerr << " done." << endl;
	  }
	}
      
      else if (command == "off")
	{
	  cerr << "Powering off cameras... please wait." << endl;
	    it = cameras.begin();
	    while(it != cameras.end())
	      {
		(*it)->setPower(false);
		cerr << (*it)->name() << " off. ";
		it ++;
	      } 
	    cerr << endl;
	}
      else if (command == "on")
	{
	  cerr << "Powering on cameras... please wait." << endl;
	  it = cameras.begin();
	  while(it != cameras.end())
	    {
	      (*it)->setPower(true);
	      cerr << (*it)->name() << " on. ";
	      it ++;
	    } 
	  cerr << endl; 
	}
      else
	{
	  cerr << "unknown command: " << command << ".  Type \"help\" for commands list." << endl;
	}
      
    }      
  


  cerr << "Returning to defaults and closing cameras" << endl;  
  it = cameras.begin();
  while(it != cameras.end())
    {
      (*it)->setPanTilt(0.0, 0.0);
      (*it)->setZoom(1);
      
      (*it)->close();
      cerr << "Closed " << (*it)->name() << endl;
      delete (*it);
      it++;
    } 
  


  
  // TODO: close and delete all camera objects 
  

  return 0;
}


// decode a string to see if it's a camera's name or short name
PanoramicCamera* findCamera(string cameraname)
{
  vector<PanoramicCamera*>::iterator it = cameras.begin();
  while(it != cameras.end())
    {
      if((*it)->name() == cameraname)
	return *it;
      else if ((*it)->shortname() == cameraname)
	return *it;
      else
	it++;
    }
  // assert: if we get here, no camera was found
  return NULL;
}


// do everything we need to set up a camera
int setUpCamera(PanoramicCamera* c) {
  
  if( ! c->init()         ) return -1;
  if( ! c->good()         ) return -2;
  if( ! c->setPower(true) ) return -3;
  
  return 0;
}

void help() {
  cerr << "" << endl;
  cerr << "The Skysphere Project, St. Olaf College." << endl;
  cerr << "camera-terminal: remotely control cameras through VISCA and capture images and panoramas" << endl;
  cerr << "usage: camera-terminal [-c configfile]" << endl;
  cerr << "" << endl;
}


int processArgs(int argc, char** args)
{
  int i = 1; // start processing at first argument
  while (i < argc)
    {
      if(strcmp(args[i], "--help") == 0)
	{
	  help();
	  exit(0);
	}	
      if(strcmp(args[i], "-c") == 0)
	{
	  // try to load a config file
	  i++;
	  if(i < argc) {
	    configFile = args[i];
	    i++;
	  } else return -10;

	}
    }
  
  return 0;
}


int readConfigFile(string filename) {
  // open file
  fstream f (filename.c_str(), fstream::in);
  if( ! f.good() ) return -1;

  // assert: file is open and ready to read
  
  char buffer[512];
  
  // read lines until no more
  while(!f.eof() && !f.fail())
    {
      // read a line
      f.getline(buffer, 512);
      
      if(f.eof() || f.fail()) break; // guard for end of file or other errors

      // guard for comments
      if(buffer[0] == '#') continue;

      string buffstring(buffer);

      // for path setting command
      if( buffstring.compare(0,5,"PATH ") == 0)
	{
	  // set up the path from this line
	  string pathstr(buffstring,5);
	  if(pathstr != "")
	    savePath = pathstr;
	}

      PanoramicCamera* pc = new PanoramicCamera;

      // load a new camera from the line we just read - if it works, add to our list of cameras
      if(0 == pc->loadLine(buffstring))
	cameras.push_back(pc);
    }

  f.close();

  return 0;
}
