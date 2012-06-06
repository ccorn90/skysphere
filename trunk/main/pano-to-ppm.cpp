// pano-to-ppm.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// main file for executable pano-to-ppm which converts a .pano file and its associated files to a .ppm image

#include "PanoImage.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

// Functions used in this program
void help();  // display usage information
int process_args(int, char**);  // loads global constants from the command line args
int process_configfile();  // loads global constants from the configfile

// Defaults for this program
const static string DEFAULT_INPUT_FILE ="";
const static string DEFAULT_OUTPUT_FILE = "./out.ppm";
const static PanoImage::blendMode DEFAULT_BLEND_MODE = PanoImage::AVERAGE;

// Globals for this program -- I know, I know, globals == ewww.
static bool promptMode = true;  // do we prompt for information?
static string configFile = "";
static string inputFile = DEFAULT_INPUT_FILE;
static string outputFile = DEFAULT_OUTPUT_FILE;
static PanoImage::blendMode blendMode = DEFAULT_BLEND_MODE;

static double thetaMin = 0.0;
static double thetaMax = 0.0;
static double phiMin   = 0.0;
static double phiMax   = 0.0;
static unsigned resolution = 0.0;

int main(int argc, char* argv[])
{
  // process arguments
  int a = process_args(argc, argv);
  if(a != 0) {
    // assert: an error happened parsing arguments
    cerr << "Error parsing arguments.  Try --help" << endl;
    return a;
  }

  // assert: properly parsed arguments

  // gather remaining information
  if(promptMode)  // for case with no config file (the default)
    {
      if(inputFile == DEFAULT_INPUT_FILE)  cerr << "Specify a source .pano file: ";  cin >> inputFile;
      cerr << endl;
      cerr << "Enter bounds for the image you'd like to decode in degrees, from -180.0 to 180.0" << endl;
      cerr << "thetaMin: ";  cin >> thetaMin;
      cerr << "thetaMax: ";  cin >> thetaMax;
      cerr << "phiMin: ";    cin >> phiMin;
      cerr << "phiMax: ";    cin >> phiMax;
      cerr << "Please enter the resolution you desire (pixels/degree): ";
      cin >> resolution;
    }
  else if (configFile != "") { // try to read config file
    int a = process_configfile();
    if(a != 0) {
      // assert: an error occured loading config file
      cerr << "Error loading config file " << configFile << endl;
      return a;
    }
  }
  
  
  // load pano file
  PanoImage pano;
  
  cerr << "Loading pano file " << inputFile << endl;
  if( pano.init(inputFile) != 0) {
    cerr << "Error occured while loading pano file." << endl;
    return -100;
  }
  
  // decode as image
  Image* img = NULL;

  cerr << "Decoding range [" << thetaMin << ", " << thetaMax << "] [" << phiMin << ", " << phiMax << "] to file " << outputFile << endl;
  img = pano.getImage(thetaMin, thetaMax, phiMin, phiMax,
		      resolution, blendMode,
		      true, outputFile, true);
  
  // check for success
  if(img == NULL)
    {
      cerr << "An error occurred while extracting a pano image" << endl; 
      return -200;
    }
  else {
    cerr << "Complete." << endl;
  }
  
  // all done!  Do clean up
  cerr << "Cleaning up... ";
  if(img) delete img;
  cerr << "done." << endl;

  return 0;
}


void help() {
  cerr << "" << endl;
  cerr << "The Skysphere Project, St. Olaf College." << endl;
  cerr << "pano-to-ppm: convert .pano files to ppm images" << endl;
  cerr << "usage: pano-to-ppm [-c configfile] [-i inputfilename] [-o outputfilename]" << endl;
  cerr << "" << endl;
}

int process_args(int argc, char* args[])
{
  int i = 1; // start processing at first argument
  while (i < argc)
    {
      if(strcmp(args[i], "--help") == 0)
	{
	  help();
	  exit(0);
	}	
      else if( (strcmp(args[i], "-c") == 0) || (strcmp(args[i], "--configfile") == 0) ) // case for -c ... get a config file
	{
	  promptMode = false; // assert: we're loading a config file
	  // try to get the name of the config file
	  i++;
	  if(i < argc) {
	    configFile = args[i];
	    i++;
	  } else return -10;
	}
      else if(strcmp(args[i], "-i") == 0)
	{
	  // try to get the name of the input file
	  i++;
	  if(i < argc) {
	    string temp(args[i]);
	    inputFile = args[i];
	    i++;
	  } else return -11;
	}
      else if(strcmp(args[i], "-o") == 0)
	{
	  // try to get the name of the input file
	  i++;
	  if(i < argc) {
	    string temp(args[i]);
	    outputFile = temp;
	    i++;
	  } else return -12;
	}
      else
	{
	  i++; // default case, just skip the argument
	}
    }
  return 0;
}


int process_configfile()
{
  fstream f (configFile.c_str(), fstream::in);  // load file for reading only
  
  if(!f.good()) return -29;
  
  // assert: config file is good for reading 


  string temp;


  // get files, first input, then output, guarded to allow for command-line options
  if(inputFile == DEFAULT_INPUT_FILE) { f >> inputFile; if(f.eof() || f.fail()) return -20; }
  else  f >> temp;

  if(outputFile == DEFAULT_OUTPUT_FILE) { f >> outputFile; if(f.eof() || f.fail()) return -21; }
  else  f >> temp;

  // get bounds for image
  f >> thetaMin; if(f.eof() || f.fail()) return -22;
  f >> thetaMax; if(f.eof() || f.fail()) return -23;
  f >> phiMin; if(f.eof() || f.fail()) return -24;
  f >> phiMax; if(f.eof() || f.fail()) return -25;
  
  // get resolution
  f >> resolution; if(f.fail()) return -25; //eof() not included as this might be eof
  f.close();
  
  return 0;
}
