// libviscaBRC300.hpp
// @author Chris Cornelius
// Created 01/10/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Special methods for controlling the Sony BRC-300 cameras through VISCA

#ifndef _VISCA_BRC300_HPP_
#define _VISCA_BRC300_HPP_

#include "libvisca.h"  // for the VISCA camera-control serial interface

// a namespace to hold methods for controllign the BRC-300 cameras through VISCA
namespace libviscaBRC300 {
  
  // MATH METHODS - convert between degrees (absolute double) and camera position (absolute uint32_t)
  // degrees are constrained to -170.0 through +170.0
  VISCA_API uint32_t degrees_to_uint(double degrees);  // according to this cameras pan/tilt law
  VISCA_API uint32_t zoom_lookup_table(int zoom);      // convert zoom ratio to zoom uint32_t
  double zoom_ratio_to_degreesTheta(int zoom); // convert zoom ratio to degrees field of view in horizontal (X or Theta) direction
  double zoom_ratio_to_degreesPhi(int zoom);   // convert zoom ratio to degrees field of view in vertical (Y or Phi) direction
  
  
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
  
}



#endif // _VISCA_BRC300_HPP
