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
#include "ImgCap_base.hpp"
#include "ImgCap_V4L2.hpp"
#include "CamCtrl_base.hpp"
#include "CamCtrl_BRC300.hpp"
#include "CamCtrl_EVIHD1.hpp"
#include "PanoImage.hpp"
#include <sstream>
#include <fstream>

// A class to represent a panoramically-scanning camera, which can create PanoImages
class PanoramicCamera
{
private:
  string _shortname;  // always defaults to "C"
  string _name;
  ImgCap_base* ic; // The ImgCap object that goes with the PanoramicCamera
  CamCtrl_base* cc; // The CamCtrl object that controls the movement and zoom of the camera.

public:
  // Constructor sets up the ImgCap and CamCtrl objects
  PanoramicCamera() {
    _name = "";
    ic = NULL;
    cc = NULL;
  }
  PanoramicCamera(string name, ImgCap_base::CapType ic_type, string device, int portIndex, CamCtrl_base::CtrlType cc_type, string serialDevice, unsigned viscaID);

  // load and write from/to lines in a file
  int loadLine(string line);
  string writeLine();

  // Destructor calls the close() method
  ~PanoramicCamera() {
    this->close(); 
    if(ic) delete ic;
    if(cc) delete cc;
  }


  
  // Accessors
  string shortname() { return _shortname; }
  void set_shortname(string sn) { _shortname = sn; }

  string name() { return _name; }
  void set_name(string n) { _name = n; }
  
  bool init(bool capture=true, bool controller=true);  // Opens required connections and preps for capturing
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
  bool setPanTilt(double theta, double phi) {
    if(cc) return cc->setPanTilt(theta,phi);
    else   return false;
  }
  bool setZoom(int ratio) {
    if(cc) return cc->setZoom(ratio);
    else return false;
  }
  bool setPower(bool on) {
    if(!cc) return false;
    if(on ) return cc->setPowerOn();
    else    return cc->setPowerOff();
  }
  bool setDzoomOff() {
    if(cc) return cc->setDzoomOff();
    else   return false; 
  }
  bool setExposureAuto(bool a) {
    if(cc) return cc->setExposureAuto(a);
    else   return false;
  }
  bool setShutter(char value) {
    if(cc) return cc->setShutter(value);
    else   return false; 
  }
  bool setIris(double fstop) {
    if(cc) return cc->setIris(fstop);
    else   return false; 
  }
  bool setGain(char value) {
    if(cc) return cc->setGain(value);
    else   return false; 
  }
  void print(std::ostream& os);
  friend std::ostream& operator<<(ostream& os, PanoramicCamera& c) {
    c.print(os);
    return os;
  }
  
};


#endif //_PANORAMIC_CAM_HPP_
