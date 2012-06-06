// PanoImage.hpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// PanoImage objects hold many Image objects and reply to queries to create panoramic images.

#ifndef _PANO_IMAGE_HPP_
#define  _PANO_IMAGE_HPP_

#include <iostream>
#include "eriolHeader.h"
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <fstream>
#include <math.h>
using std::vector;  using std::string;  using std::pair;
using std::stringstream;  using std::fstream; using std::size_t;

// for openMP parallel flags -- define this variable with -D when you compile
#ifdef __SKYSPHERE_OPEN_MP_FLAG__
#include <omp.h>
#endif

// functions to do data transformations on images
int flipByteOrder_gb(Image& img);
int YUVtoRGB(Image& img);

// Associates an Image with other important data and respresents one image used to generate the panorama
// Design use: only import an Image object by calling load() when you actually need the Image
struct PanoElement {
public:
  Image* image;     // one of Olaf's image objects
  string filename;
  double centerTheta, centerPhi;  // values for angle of ray at center of image
  double widthTheta,  widthPhi;   // values for angular field of view of this image
  
private:
  void setDefaults() { image = NULL; filename = ""; centerTheta=centerPhi=widthTheta=widthPhi=0.0; }

public:
  
  // constructor and destructor do basic pointer management for image.
  PanoElement() { setDefaults(); }
    PanoElement(const PanoElement& src); // copy constructor - defined in PanoImage.cpp - Does not load image!
  PanoElement& operator=(const PanoElement& src);  // defined in PanoImage.cpp - Does not load image!
  ~PanoElement() { if(image) delete image; }

  // methods for helping calculate points within this element
  double getKValuePhi();   // TODO - replace with local vars calculated in init();
  double getKValueTheta();
  bool containsPoint(double theta, double phi);
  bool isValidLoc(PixelLoc loc);
  bool isDistanceFromEdges(PixelLoc loc, double fraction);
  PixelLoc findPixelLocFromCenter (double thetaPrime, double phiPrime); // theta and phi from centerof the element, not total!


  // is this PanoElement okay to use?
  bool good() { return !(image == NULL); }

  // write a line for a pano file describing this element. Note: defined in PanoImage.cpp
  string write();
  
  // read a line for a pano file describing this element - Does not load image! Note: defined in PanoImage.cpp
  int init(const string line, const string contextDir=".");

  // load or reload image from filename, using memory-mapping!  Returns 0 on success.
  int reload() {
    if(image) delete image;
    image = new Image(filename.c_str(), true); // use memory map
    if(image) return 0;  // make sure we allocated!
    else      return 1;
  }
  
  // only loads if needed
  int load() {
    if(image) return 0;
    // assert: image is not loaded
    image = new Image(filename.c_str(), true); // use memory map
    if(image) return 0;  // make sure we allocated!
    else      return 1;
  }

  // utility methods
  void print(ostream& os);    // prints stats of this PanoElement to the given stream
  //friend ostream& operator<<(ostream& os, PanoElement& p) { p.print(os); return os; } // friend function for printing using <<
};


// PanoImage objects store a panoramic image covering all or a section of
// the sky. They can be written to and read from their own file format, and
// an be projected into a 2D image based on whatever angle of view desired.
// PanoImages work by combining a bunch of Image objects (loaded from files
// on disk) and computing the responses to requests on-the-fly.
// Holds a vector of Image objects which are used to reply to queries about
// the inside of a panoramic, spherical data structure.

// for the getPixel_weighted method -- default mask to use on images
const static string DEF_MASK_FILE = "/home/cg/palantir/regents/skysphere/data/mask.ppm";

class PanoImage {
private:
  vector<PanoElement> elements;  // holds all the images involved in the panorama
  int loadElements(vector<size_t> & indexes);  // loads all the images in the vector given
  
  // Private methods for various strategies of blending
  Color getPixel_select(double theta, double phi); // for the SELECT option
  Color getPixel_average(double, double);   // for the AVERAGE option
  Color getPixel_masked(double, double);   // for the MASKED option
  Color getPixel_weighted(double, double);   // for the WEIGHTED option
  // TODO: add more options for blending modes
    
  Image maskImg;   // for getPixel_weighted.
  
public:
  ~PanoImage() { } // Destructor - since vector manages all our elements, there's nothing to do
  PanoImage() : maskImg(DEF_MASK_FILE.c_str(), false) { }

  // Initialization methods
  int init(string filename);  // from pano file
  int init(PanoImage& src);   // from another PanoImage implicitly (and define operator=)
  
  
  // Other key object management methods
  int writePanoFile(string filename);  // write object out to a pano file
  
  // Input of data
  int addImage(string filename, double centerTheta, double centerPhi,
	       double widthTheta, double widthPhi);
  
  // Extraction of data
  enum blendMode { SELECT, AVERAGE, MASKED, WEIGHTED };  // TODO: add more options

  Color getPixel(double theta, double phi, blendMode mode=WEIGHTED);
  Image* getImage(double thetaMin, double thetaMax, double phiMin, double phiMax,
		  unsigned pixelsPerDegree, blendMode mode=SELECT,
		  bool useMmap=false, string mMapFilename="out.ppm",
		  bool verbose=true);

  
  

  // helper methods
  int findNearestPanoElement (double theta, double phi);
  vector<size_t> getContainingElements(double theta, double phi);
  size_t getPPMFilenames(string* & dest); // puts the names of all the image files in this PanoImage into the given string


  // Utility methods
  void print(ostream& os);    // prints stats of this PanoImage to the given stream
  friend ostream& operator<<(ostream& os, PanoImage& p) { p.print(os); return os; } // friend function for printing using <<
};


#endif // _PANO_IMAGE_HPP_
