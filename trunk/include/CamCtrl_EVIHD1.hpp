// CamCtrl_EVIHD1.hpp
// @author Chris Cornelius
// Created 01/23/2012
// ATP 2012, Skysphere Project St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// CamCtrl_EVIHD1 class -- a CameraControl class for the EVI-HD1 highdef camera

#ifndef _CAM_CTRL_EVIHD1_HPP_
#define _CAM_CTRL_EVIHD1_HPP_

#include "CamCtrl_base.hpp"
#include "libviscaEVIHD1.hpp" // for the namespace to control the BRC-300 camera

class CamCtrl_EVIHD1 : public CamCtrl_base {
public:
  // Constructor overrides
  CamCtrl_EVIHD1() : CamCtrl_base(CamCtrl_base::EVIHD1) { }
  CamCtrl_EVIHD1(string serialDevice, int viscaID) : CamCtrl_base(serialDevice, viscaID, CamCtrl_base::EVIHD1) { }
  
  // Camera-specific methods
  // Set absolute values for zoom, focus, pan, tilt
  virtual bool setPan(double theta);
  virtual bool setTilt(double phi);
  virtual bool setPanTilt(double theta, double phi);
  virtual bool setZoom(uint32_t absolute); // use a direct indexing value
  virtual bool setZoom(int ratio);  // choose a zoom ratio: 1x 2x 3x ... 12x
  // TODO: setFocus(uint32_t absolute);  // use a direct indexing value
  // TODO: setFocus(int depth);  // choose a focal distace in centimeters: inf=10000 500 200 120 80 50 20 11 6c 2c 1
  
  // Power camera on and off
  virtual bool setPowerOn();
  virtual bool setPowerOff();
  
  // Set absolute values for exposure
  virtual bool setDzoomOff();
  virtual bool setExposureAuto(bool a);
  virtual bool setShutter(char value);
  virtual bool setIris(double fstop);
  virtual bool setGain(char value);

  // TODO: other stuffs
  
};

#endif // _CAM_CTRL_EVIHD1_HPP_
