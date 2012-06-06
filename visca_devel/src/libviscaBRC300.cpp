// libviscaBRC300.cpp
// @author Chris Cornelius
// Created 01/10/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Special methods for controlling the Sony BRC-300 cameras through VISCA

#include "libviscaBRC300.hpp"

uint32_t libviscaBRC300::degrees_to_uint(double degrees) // convert according to this cameras pan/tilt law
{
  // Degrees should be constrained to the -170 to +170 case
  if(degrees < -170.0) degrees = -170.0;
  if(degrees >  170.0) degrees =  170.0;
  
  // Note: one degree of motion is 0xD0 in the uint case
  double oneDegree = 0xD0;
  
  // for positive case (motion defined to the RIGHT)
  if(degrees > 0)
    {
      // initial calculation using excel: int value = -208.31*(degrees right) + 65536.63
      // TODO: replace with lookup table and interpolation according to Sony chart
      
      degrees *= -208.31;  // do transform
      degrees += 65536.63;
      
      uint32_t r = ( ((uint32_t) degrees) | 0x000F0000 ) ; // cast and return (masking with F in proper location for this number)   
      
      if(r < 0x000F75A8) return 0x000F75A8; // hard bound stop // TODO: remove
      return r;
    }
  
  // for negative case
  if(degrees <= 0)
    {
      // initial calculation using excel: int value = 209.57*(degrees left) + 19.84
      // TODO: replace with lookup table and interpolation according to Sony chart
      
      degrees *= -1.0;  // flip to positive
      
      degrees *= 209.57;  // do transform
      degrees += 19.84;
      
      uint32_t r = (uint32_t) degrees; // cast and return 
      if(r > 0x00008A58) return 0x0008A58; // hard bound stop // TODO: remove
      return r;

    }
}


// Maps a ratio of zooming (1x 2x .. 12x) to the appropriate absolute value
uint32_t libviscaBRC300::zoom_lookup_table(int zoom)
{
  if(zoom >= 12) return 0x4000;
  if(zoom == 11) return 0x3EBB;
  if(zoom == 10) return 0x3D43;
  if(zoom ==  9) return 0x3B8B;
  if(zoom ==  8) return 0x3988;
  if(zoom ==  7) return 0x3724;
  if(zoom ==  6) return 0x343D;
  if(zoom ==  5) return 0x3099;
  if(zoom ==  4) return 0x2BC9;
  if(zoom ==  3) return 0x24E2;
  if(zoom ==  2) return 0x1982;
  
  // assert: default case (zoom <= 1)
  return 0x0000;
}


// TODO: Find out proper values from geometric precision team
double libviscaBRC300::zoom_ratio_to_degreesTheta(int zoom) // convert zoom ratio to degrees field of view in horizontal (X or Theta) direction
{
  if(zoom >= 12) return 5.03;
  if(zoom == 11) return 5.03;
  if(zoom == 10) return 5.03;
  if(zoom ==  9) return 5.03;
  if(zoom ==  8) return 5.03;
  if(zoom ==  7) return 5.03;
  if(zoom ==  6) return 5.03;
  if(zoom ==  5) return 5.03;
  if(zoom ==  4) return 5.03; // real val 1/13
  if(zoom ==  3) return 9.10;
  if(zoom ==  2) return 9.10; // real val 1/13
  
  // assert: default case (zoom <= 1)
  return 19.1 ; // real val 1/13
}

// TODO: Find out proper values from geometric precision team
double libviscaBRC300::zoom_ratio_to_degreesPhi(int zoom)   // convert zoom ratio to degrees field of view in vertical (Y or Phi) direction
{
  if(zoom >= 12) return 3.73;
  if(zoom == 11) return 3.73;
  if(zoom == 10) return 3.73;
  if(zoom ==  9) return 3.73;
  if(zoom ==  8) return 3.73;
  if(zoom ==  7) return 3.73;
  if(zoom ==  6) return 3.73;
  if(zoom ==  5) return 3.73;
  if(zoom ==  4) return 3.73; // real val 1/13
  if(zoom ==  3) return 6.84;
  if(zoom ==  2) return 6.84; // real val 1/13
  
  // assert: default case (zoom <= 1)
  return 14.40 ; // real val 1/13
}




// set absolute pan/tilt position
VISCA_API uint32_t libviscaBRC300::set_pantilt_absolute_position(VISCAInterface_t *iface, VISCACamera_t *camera, uint32_t speed, uint32_t pan_position, uint32_t tilt_position)
{
  // Set up the packet we'll send
  VISCAPacket_t packet;
  _VISCA_init_packet(&packet);

  // Used proper codes for sending to camera
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x06);
  _VISCA_append_byte(&packet, 0x02);
  _VISCA_append_byte(&packet, 0x18 ); // rate of motion: 0x18 is fastest
  _VISCA_append_byte(&packet, 0x00);

  
  // needed to have five bytes for pan, four bytes for tilt
  _VISCA_append_byte(&packet, (pan_position & 0xf0000) >> 16);
  _VISCA_append_byte(&packet, (pan_position & 0x0f000) >> 12);
  _VISCA_append_byte(&packet, (pan_position & 0x00f00) >>  8);
  _VISCA_append_byte(&packet, (pan_position & 0x000f0) >>  4);
  _VISCA_append_byte(&packet,  pan_position & 0x0000f       );

  _VISCA_append_byte(&packet, (tilt_position & 0xf000) >> 12);
  _VISCA_append_byte(&packet, (tilt_position & 0x0f00) >> 8);
  _VISCA_append_byte(&packet, (tilt_position & 0x00f0) >> 4);
  _VISCA_append_byte(&packet, tilt_position & 0x000f);
 

  //_VISCA_append_byte(&packet, 0xFF); // it is NOT REQUIRED to add the terminating 0xff byte


  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}



// set absolute zoom position
VISCA_API uint32_t libviscaBRC300::set_zoom_absolute_position(VISCAInterface_t* iface, VISCACamera_t* camera, uint32_t zoom)
{
  // minimum and maximum zoom values for the BRC300
  uint32_t min = 0x00000000;
  uint32_t max = 0x00004000;

  // clamp zoom to appropriate values
  if(zoom > max) zoom = max;
  if(zoom < min) zoom = min;
  
  // standard libvisca function works with the BRC300
  return VISCA_set_zoom_value(iface, camera, zoom);
}



VISCA_API uint32_t libviscaBRC300::set_camera_power(VISCAInterface_t* iface, VISCACamera_t* camera, bool powerStatus)
{
  VISCAPacket_t packet;

  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x01);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x00);

  if(powerStatus)
    _VISCA_append_byte(&packet, 0x02); // power ON with byte 0x02
  else
    _VISCA_append_byte(&packet, 0x03); // power OFF with byte 0x03

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}
