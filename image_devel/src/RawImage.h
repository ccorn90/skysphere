//RawImage.h
//Ian McGinnis, Charles Nye, Chris Cornelius
//Skysphere
//Saint Olaf College

#ifndef RAWIMAGE_H
#define RAWIMAGE_H

#include <vector>
using std::vector;

struct RawImage
{
  int width; //a user supplied value, for our camera it is 640 (I think)
  int height; //a user supplied value, for our camera it is 480 (I think)
  int range; //normally 255 for .ppm format
  int format; //an int 1-6 which corrosponds to P1-P6. P1-P6 are the .ppm format options
  vector<int> data; //stores the data for the actual picture. May later become a vector of pixel structs, not Ints.
//a constructor for our (St. Olaf CS) default image sizes. Also .ppm standard format and range.
  RawImage()  {
  width=640;
  height=480;
  range=255;
  format=6;
  //data is uninitialized
  }

};

#endif // RAWIMAGE_H
