// shutdown-cameras.cpp
// @author Chris Cornelius
// Created 01/24/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Shuts down the given cameras (takes a config file). Adapted from camera-terminal.cpp

#include "PanoramicCamera.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

// global methods
int readConfigFile(string filename);
int processArgs(int, char**);

// defaults for the program
const static string DEFAULT_CONFIG_FILE = "./data/ctconfig";

// Globals for the program
vector<PanoramicCamera*> cameras;
string configFile = DEFAULT_CONFIG_FILE;

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

  // Load config file
  if(0 == readConfigFile(configFile)) {
    cerr << "Read config file " << configFile << endl;
  }
  else {
    cerr << "Unable to read config file " << configFile << endl;
    cerr << "Try running with [-c configfile] and specifying the settings you want." << endl;
    return 0;
  }
  
  
  // initialize and power off all cameras
  cerr << "Powering off " << cameras.size() << " cameras... please wait." << endl;
  vector<PanoramicCamera*>::iterator it = cameras.begin();
  it = cameras.begin();
  while(it != cameras.end())
    {
      cerr << (*it)->name() << "... ";
      if( (*it)->init() && (*it)->good() )
	{
	  (*it)->setPower(false);
	  cerr << " off." << endl;
	}
      else {
	cerr << " Error: Could not access camera";
      }

      it ++;
    }
  
  // close all cameras
  cerr << "Closing cameras" << endl;  
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
  
  return 0;
}



void help() {
  cerr << "" << endl;
  cerr << "The Skysphere Project, St. Olaf College." << endl;
  cerr << "shutdown-cameras: shuts down all cameras found in config file" << endl;
  cerr << "usage: shutdown-cameras [-c configfile]" << endl;
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

      PanoramicCamera* pc = new PanoramicCamera;

      // load a new camera from the line we just read - if it works, add to our list of cameras
      if(0 == pc->loadLine(buffstring))
	cameras.push_back(pc);
    }

  f.close();

  return 0;
}
