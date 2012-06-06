// ImgCap_base.hpppp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/23/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Superclass for classes used to capture images from video devices... derive all ImgCap objects from this, please

#ifndef _IMGCAP_HPP_
#define _IMGCAP_HPP_

//library used for interfacing with Prof. Olaf's eriol program
#include "eriolHeader.h"
#include <cstring>

class ImgCap_base
{
public:
  // the different types that this ImgCap can be
  enum CapType { 
    ERROR = -1,
    V4L2 = 1,
    BLACKMAGIC = 2
  };

protected:
  // a variable for controlling error printing in the cpp file associated with these objects
  bool verboseErrorPrinting;
  
  char *dev_name;
  int device_input_index;
  bool cameraOn;

  CapType _type; // What type is this object?

 public:
  ImgCap_base(const char* device, int deviceIndex, ImgCap_base::CapType t) {
    _type = t;
    dev_name = new char[strlen(device)];
    strcpy(dev_name,device);
    device_input_index = deviceIndex;
    cameraOn = false;
    verboseErrorPrinting = false;
  }

  ~ImgCap_base() {
    closeConnection();
  }
  
  // accessors
  void set_type(ImgCap_base::CapType t) { _type = t; }  
  ImgCap_base::CapType type() { return _type; }
  void set_device(const char* dev) {
    if(dev_name) delete[] dev_name;
    dev_name = new char[strlen(dev)];
    strcpy(dev_name, dev);
  }
  char* device() { return dev_name; }
  void set_index(int ind) {
    device_input_index = ind;
  }
  int index() { return device_input_index; }
  bool verbose() { return verboseErrorPrinting; }
  void set_verbose(bool v) { verboseErrorPrinting = v; }
  
  // Methods to override!
  virtual void grabImageToFile() { } //saves images with fileNumber, camera, and time
  virtual int grabImageToPPM(const char * fileName) { } //save an image w/given filename
  virtual bool openConnection() { cameraOn = true; } // prepare to receive Image structs from the device
  virtual bool closeConnection()  // elegantly break connection to the device
  {
    if(cameraOn) cameraOn = false; 
    return cameraOn;
  }
  virtual Image grabImage() { } 
  virtual bool good() {
    return cameraOn;
  }  

friend ostream& operator<< (ostream& os, ImgCap_base::CapType cap) {
  if(cap == ImgCap_base::V4L2) os << "V4L2";
  else if(cap == ImgCap_base::BLACKMAGIC) os << "BLACKMAGIC";
  else os << "ERROR";
  return os;
}

};




#endif // _IMGCAP_HPP_
