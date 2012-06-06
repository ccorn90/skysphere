// CamCtrl_BRC300.cpp
// @author Chris Cornelius
// Created 01/20/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike

// Holds definitions for methods controlling the Sony BRC-300 camera

#include "CamCtrl_BRC300.hpp"

bool CamCtrl_BRC300::setPan(double theta)
{
  // TODO: implement
}

bool CamCtrl_BRC300::setTilt(double phi)
{
  // TODO: implement
}


bool CamCtrl_BRC300::setExposureAuto(bool a)
{
  if(a) {
    return (libviscaBRC300::set_auto_exposure(&_iface, &_vcamera) == VISCA_SUCCESS);
  }
  // assert: turn on manual exposure
  return (libviscaBRC300::set_manual_exposure(&_iface, &_vcamera) == VISCA_SUCCESS);
}


bool CamCtrl_BRC300::setDzoomOff() 
{
  return (libviscaBRC300::set_dzoom_off(&_iface, &_vcamera) == VISCA_SUCCESS);
}

bool CamCtrl_BRC300::setShutter(char value ) // 0x02 = 1/3 sec, 0x06 = normal, 0x15 = fastest
{
  return (libviscaBRC300::set_absolute_shutter(&_iface, &_vcamera, value) == VISCA_SUCCESS);
}

bool CamCtrl_BRC300::setIris(double fstop)
{
  return (libviscaBRC300::set_absolute_iris(&_iface, &_vcamera, libviscaBRC300::iris_lookup_table(fstop)) == VISCA_SUCCESS);
}

bool CamCtrl_BRC300::setGain(char value) // 0x00 = -3db, 0x01 = normal, 0x07 = max
{
  return (libviscaBRC300::set_absolute_gain(&_iface, &_vcamera, value) == VISCA_SUCCESS);
}

bool CamCtrl_BRC300::setPanTilt(double theta, double phi)
{
  // convert theta and phi into camera range numbers
  uint32_t pan  = libviscaBRC300::degrees_to_uint_pan(theta);
  uint32_t tilt = libviscaBRC300::degrees_to_uint_tilt(phi);

  uint32_t speed = 0x00000018; // maximum speed, captain.
  
  // clamp to acceptable range for camera's pan and tilt
  if(pan > 0x08a58  && pan <= 0xf75a8) pan = 0xf75a8;
  if(pan > 0xffffff) pan = 0xffffff;
  if(tilt > 0x1869 && tilt < 0xA000) tilt = 0x1869;
  if(tilt > 0xA000 && tilt < 0xB6C2) tilt = 0xB6C2;
  if(tilt > 0xffff) tilt = 0xffff;
 
  // send command and wait for reply
  if( libviscaBRC300::set_pantilt_absolute_position(&_iface, &_vcamera, speed, pan, tilt)
      != VISCA_SUCCESS )
    return false;
  else
    return true;
}


bool CamCtrl_BRC300::setZoom(uint32_t absolute) // for use with direct indexing value
{
  if( libviscaBRC300::set_zoom_absolute_position(&_iface, &_vcamera, absolute) != VISCA_SUCCESS )
    return false;
  else
    return true;
}



bool CamCtrl_BRC300::setZoom(int ratio)  // choose a zoom ratio: 1x 2x 3x ... 12x
{
  // use lookup table for the appropriate camera
  uint32_t absolute = libviscaBRC300::zoom_lookup_table(ratio); 
  
  // use the appropriate BRC300 method
  if( libviscaBRC300::set_zoom_absolute_position(&_iface, &_vcamera, absolute) != VISCA_SUCCESS )
    return false;
  else
    return true;
}


bool CamCtrl_BRC300::setPowerOn()
{
  // use the appropriate BRC300 method
  if( libviscaBRC300::set_camera_power(&_iface, &_vcamera, true) != VISCA_SUCCESS)
    return false;
  else
    return true;
}

bool CamCtrl_BRC300::setPowerOff()
{
  // use the appropriate BRC300 method
  if( libviscaBRC300::set_camera_power(&_iface, &_vcamera, false) != VISCA_SUCCESS)
    return false;
  else
    return true;
}
