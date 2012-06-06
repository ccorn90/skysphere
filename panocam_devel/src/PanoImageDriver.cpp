// PanoImageDriver.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Driver for PanoImage class.

#include "PanoImage.hpp"

#include <iostream>
#include <string>
#include <fstream>

#include "PanoImage.hpp"
using namespace std;

const static string DEFAULT_PANO_FILE ="/home/cg/palantir/regents/skysphere/data/overlap2x/pano.pano";
const static string DEFAULT_OUTPUT_FILE = "./data/out.ppm";

int main(int argc, char* argv[])
{  
  // variables for operation of program
  double thetaMin = -60.0;
  double thetaMax = 60.0;
  double phiMin   = -30.0;
  double phiMax   = 30.0;
  unsigned pixPerDegree = 50;
  

  string panoFile = DEFAULT_PANO_FILE;
  string outputFile = DEFAULT_OUTPUT_FILE;
  
  // set up filenames from arguments, if possible
  if(argc > 1)
    {
      string temp (argv[1]);
      panoFile = temp;
    }
  if(argc > 2)
    {
      string temp (argv[2]);
      outputFile = temp;
    }

 
  string contextDir = panoFile.substr(0, panoFile.find_last_of("/"));
  cerr << "Context is " << contextDir << endl;
  
  cerr << "Reading pano file " << panoFile << endl;
  PanoImage pi;
  if(0!=pi.init(panoFile))
    {
      cerr << "Unable to init from file" << endl;
      return -10;
    }
 
  // assert: read the PanoImage okay
  cerr << "Successfully read in " << pi << endl;
  
  // extract an image over a given range... memory-mapping has it end up in a file, too.
  cerr << "Outputting selected image to memory-mapped file " << outputFile << endl;  
  Image* i = pi.getImage(thetaMin, thetaMax, phiMin, phiMax, pixPerDegree, PanoImage::WEIGHTED, true, outputFile);

  if(!i) { cerr << "Could not create image object... abort" << endl; return -12; }

  // clean up
  cerr << "Complete." << endl;
  if(i) delete i;

  return 0;
}
