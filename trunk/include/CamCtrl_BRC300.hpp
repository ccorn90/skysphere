// CamCtrl_BRC300.hpp
// @author Chris Cornelius
// Created 01/20/2012
// ATP 2012, Skysphere Project St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// CamCtrl_BRC300 class -- a CameraControl class for the BRC-300 camera

#ifndef _CAM_CTRL_BRC300_HPP_
#define _CAM_CTRL_BRC300_HPP_

#include "CamCtrl_base.hpp"
#include "libviscaBRC300.hpp" // for the namespace to control the BRC-300 camera

class CamCtrl_BRC300 : public CamCtrl_base {
public:
  // Constructor overrides
  CamCtrl_BRC300() : CamCtrl_base(CamCtrl_base::BRC300) { }
  CamCtrl_BRC300(string serialDevice, int viscaID) : CamCtrl_base(serialDevice, viscaID, CamCtrl_base::BRC300) { }
  
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
  virtual bool setExposureAuto(bool a);  // false = manual, true = auto
  virtual bool setDzoomOff();
  virtual bool setShutter(char value ); // 0x02 = 1/3 sec, 0x06 = normal, 0x15 = fastest
  virtual bool setIris(double fstop);
  virtual bool setGain(char value); // 0x00 = -3db, 0x01 = normal, 0x07 = max

  // TODO: other stuffs
  
};

#endif // _CAM_CTRL_BRC300_HPP_
