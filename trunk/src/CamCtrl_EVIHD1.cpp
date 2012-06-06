// CamCtrl_EVIHD1.cpp
// @author Chris Cornelius
// Created 01/23/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike

// Holds definitions for methods controlling the Sony EVI-HD1 camera

#include "CamCtrl_EVIHD1.hpp"

bool CamCtrl_EVIHD1::setPan(double theta)
{
  // TODO: implement
}

bool CamCtrl_EVIHD1::setTilt(double phi)
{
  // TODO: implement
}

bool CamCtrl_EVIHD1::setExposureAuto(bool a)
{
  if(a) {
    return (libviscaEVIHD1::set_auto_exposure(&_iface, &_vcamera) == VISCA_SUCCESS);
  }
  // assert: must be not a
  return (libviscaEVIHD1::set_manual_exposure(&_iface, &_vcamera) == VISCA_SUCCESS);
}

bool CamCtrl_EVIHD1::setDzoomOff() 
{
  return libviscaEVIHD1::set_dzoom_off(&_iface, &_vcamera);
}

bool CamCtrl_EVIHD1::setShutter(char value)
{
  return libviscaEVIHD1::set_absolute_shutter(&_iface, &_vcamera, value);
}

bool CamCtrl_EVIHD1::setIris(double fstop)
{
  return libviscaEVIHD1::set_absolute_iris(&_iface, &_vcamera, libviscaEVIHD1::iris_lookup_table(fstop));
}

bool CamCtrl_EVIHD1::setGain(char value)
{
  return libviscaEVIHD1::set_absolute_gain(&_iface, &_vcamera, value);
}


// TODO: Make so it's not a clone of the BRC300 method
bool CamCtrl_EVIHD1::setPanTilt(double theta, double phi)
{
  if( ! _open ) return false;
  // convert theta and phi into camera range numbers
  uint32_t pan  = libviscaEVIHD1::degrees_to_uint_pan(theta);
  uint32_t tilt = libviscaEVIHD1::degrees_to_uint_tilt(phi);

  uint32_t speed = 0x00000018; // maximum speed, captain.
  
  // clamp to acceptable range for camera's pan and tilt
  if(pan > 0x08a58  && pan <= 0xf75a8) pan = 0xf75a8;
  if(pan > 0xffffff) pan = 0xffffff;
  if(tilt > 0x1869 && tilt < 0xA000) tilt = 0x1869;
  if(tilt > 0xA000 && tilt < 0xB6C2) tilt = 0xB6C2;
  if(tilt > 0xffff) tilt = 0xffff;
 
  // send command and wait for reply
  if( libviscaEVIHD1::set_pantilt_absolute_position(&_iface, &_vcamera, speed, pan, tilt)
      != VISCA_SUCCESS )
    return false;
  else
    return true;
}

// TODO: Make so it's not a clone of the BRC300 method
bool CamCtrl_EVIHD1::setZoom(uint32_t absolute) // for use with direct indexing value
{
  if( libviscaEVIHD1::set_zoom_absolute_position(&_iface, &_vcamera, absolute) != VISCA_SUCCESS )
    return false;
  else
    return true;
}


// TODO: Make so it's not a clone of the BRC300 method
bool CamCtrl_EVIHD1::setZoom(int ratio)  // choose a zoom ratio: 1x 2x 3x ... 12x
{
  // use lookup table for the appropriate camera
  uint32_t absolute = libviscaEVIHD1::zoom_lookup_table(ratio); 
  
  // use the appropriate EVIHD1 method
  if( libviscaEVIHD1::set_zoom_absolute_position(&_iface, &_vcamera, absolute) != VISCA_SUCCESS )
    return false;
  else
    return true;
}

// TODO: Make so it's not a clone of the BRC300 method
bool CamCtrl_EVIHD1::setPowerOn()
{
  // use the appropriate EVIHD1 method
  if( libviscaEVIHD1::set_camera_power(&_iface, &_vcamera, true) != VISCA_SUCCESS)
    return false;
  else
    return true;
}

// TODO: Make so it's not a clone of the BRC300 method
bool CamCtrl_EVIHD1::setPowerOff()
{
  // use the appropriate EVIHD1 method
  if( libviscaEVIHD1::set_camera_power(&_iface, &_vcamera, false) != VISCA_SUCCESS)
    return false;
  else
    return true;
}
