// libviscaBRC300.hpp
// @author Chris Cornelius
// Created 01/23/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Special methods for controlling the Sony EVI-HD1 cameras through VISCA

#ifndef _VISCA_EVIHD1_HPP_
#define _VISCA_EVIHD1_HPP_

#include "libvisca.h"  // for the VISCA camera-control serial interface

// a namespace to hold methods for controllign the BRC-300 cameras through VISCA
namespace libviscaEVIHD1 {
  
  // MATH METHODS - convert between degrees (absolute double) and camera position (absolute uint32_t)
  // degrees are constrained to -170.0 through +170.0
  VISCA_API uint32_t degrees_to_uint_pan(double degrees);  // according to this cameras pan/tilt law
  VISCA_API uint32_t degrees_to_uint_tilt(double degrees);  // according to this cameras pan/tilt law
  VISCA_API uint32_t zoom_lookup_table(int zoom);      // convert zoom ratio to zoom uint32_t
  double zoom_ratio_to_degreesTheta(int zoom); // convert zoom ratio to degrees field of view in horizontal (X or Theta) direction
  double zoom_ratio_to_degreesPhi(int zoom);   // convert zoom ratio to degrees field of view in vertical (Y or Phi) direction
  char iris_lookup_table(double f);  // select an f-stop  
  
  // set absolute pan/tilt position
  VISCA_API uint32_t set_pantilt_absolute_position(VISCAInterface_t*, VISCACamera_t*,
						   uint32_t speed, uint32_t pan_position,
						   uint32_t tilt_position);
  
  // set absolute zoom position
  VISCA_API uint32_t set_zoom_absolute_position(VISCAInterface_t*, VISCACamera_t*,
						uint32_t zoom);
  

  // set camera power... powerStatus=true for ON
  VISCA_API uint32_t set_camera_power(VISCAInterface_t*, VISCACamera_t*,
				    bool powerStatus);
  
  
  // turn off digital zoom. A good thing to do.
  VISCA_API uint32_t set_dzoom_off(VISCAInterface_t*, VISCACamera_t*);

  // turn off the menu screen. A good thing to do.
  VISCA_API uint32_t set_menu_off(VISCAInterface_t*, VISCACamera_t*);

  // change the exposure mode
  VISCA_API uint32_t set_auto_exposure(VISCAInterface_t*, VISCACamera_t*);
  VISCA_API uint32_t set_manual_exposure(VISCAInterface_t*, VISCACamera_t*);

  // manually set the shutter speed -- 0x00 = 1/2 sec, 0x06 = normal, 0x15 = fastest
  VISCA_API uint32_t set_absolute_shutter(VISCAInterface_t*, VISCACamera_t*, char value);

  // manually set the fstop -- use mapping method
  VISCA_API uint32_t set_absolute_iris(VISCAInterface_t*, VISCACamera_t*, char value);

  // manually set the gain -- 0x00 = -3db, 0x01 = normal, 0x07 = max
  VISCA_API uint32_t set_absolute_gain(VISCAInterface_t*, VISCACamera_t*, char value);


}



#endif // _VISCA_EVIHD1_HPP
