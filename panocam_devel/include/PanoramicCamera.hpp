// PanoramicCamera.hpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// ATP 2012, Skysphere Project
// St. Olaf College
// Created 01/06/2012
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// PanoramicCamera class manages a camera.  Used to generate PanoImages using an attached camera.

#ifndef _PANORAMIC_CAM_HPP_
#define _PANORAMIC_CAM_HPP_

#include "eriolHeader.h"
#include "ImageCapture.h"
#include "CameraController.hpp"
#include "PanoImage.hpp"
#include "libviscaBRC300.hpp"
#include <sstream>
#include <fstream>

// A class to represent a panoramically-scanning camera, which can create PanoImages
class PanoramicCamera
{
private:
  string _name;
  ImageCapture ic; // The ImageCapture object that goes with the PanoramicCamera
  CameraController cc; // The CameraController object that controls the movement and zoom of the camera.

public:
  // Constructor sets up the ImageCapture and CameraController objects
  PanoramicCamera(string name, string device, int portIndex, string serialDevice, unsigned viscaID) : ic((char*)device.c_str(), portIndex), cc(serialDevice, viscaID) , _name(name) { }

  // Destructor calls the close() method
  ~PanoramicCamera() { this->close(); }
  
  // Accessors
  string name() { return _name; }
  void set_name(string n) { _name = n; }
  
  bool init();  // Opens required connections and preps for captureing
  bool close(); // Closes all connections and preps for destruction
  bool good();  // Returns true if the camera is ready to capture an image

  // Captures a panoramic image spanning the given section of the sky, and saves the ppm files into the given directory
  int grabPano(string destinationDirectory, double ThetaMin, double ThetaMax, double PhiMin, double PhiMax,
	       double overlap=0.3, int zoomRatio=2, int picsPerLocation=1,
	       bool verbose=false, string panofilename="pano"); 
  
  // Captures one frame from the camera and saves it to disk
  int grabImageToPPM(string filename, double theta, double phi, int zoomRatio);
  int grabImageToPPM(string filename);
  
  // TODO: Captures one frame from the camera and returns an image object

  // Direct camera control methods
  bool setPanTilt(double theta, double phi) { return cc.setPanTilt(theta,phi); }
  bool setZoom(int ratio) { return cc.setZoom(ratio); }
  
  
  
};


#endif //_PANORAMIC_CAM_HPP_
