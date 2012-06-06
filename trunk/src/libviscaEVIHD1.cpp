// libviscaEVIHD1.hpp
// @author Chris Cornelius
// Created 01/23/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Special methods for controlling the Sony EVI-HD1 cameras through VISCA... best when compiled into library libviscaEVIHD1.a

#include "libviscaEVIHD1.hpp"

// TODO: adapt this method so it's not a clone of the BRC300 version
uint32_t libviscaEVIHD1::degrees_to_uint_pan(double degrees) // convert according to this cameras pan law
{
  // Degrees should be constrained to the -170 to +170 case
  if(degrees < -170.0) degrees = -170.0;
  if(degrees >  170.0) degrees =  170.0;
 
  uint32_t r = 0x0000;
  
  // for positive case (motion defined to the RIGHT)
  if(degrees > 0)
    {
      degrees *= -208.3000;  // do transform
      degrees += 65536.000;
      
      r = ( ((uint32_t) degrees) | 0x000F0000 ) ; // cast and return (masking with F in proper location for this number)   
      
      if(r < 0x000F75A8) return 0x000F75A8; // hard bound stop // TODO: remove
    }
  
  // for negative case
  else if(degrees <= 0)
    {
      degrees *= -1.0;  // flip to positive
      
      degrees *= 208.3;  // do transform
      degrees += 00.00;
      
      r = (uint32_t) degrees; // cast and return 
      if(r > 0x00008A58) return 0x0008A58; // hard bound stop // TODO: remove
    }
  return r;
}


// 1/17: Sony's interface table might be backwards?  We were getting messed-up results, and exchanging the positive and negative cases fixed the problem.  So sending a hex number which is supposed to be -45 according to the white paper yields proper results (a +45 rotation).  Adapted this function accordingly.
// TODO: adapt this method so it's not a clone of the BRC300 version
uint32_t libviscaEVIHD1::degrees_to_uint_tilt(double degrees) // convert according to this cameras tilt law
{
  if(degrees < -30.0) degrees = -30.0;
  if(degrees >  90.0) degrees =  90.0;
  
  uint32_t r = 0x0000;

  // for positive case (motion defined to the RIGHT)
  if(degrees > 0)
    {
      degrees *= -208.35;  // do transform
      degrees += 65536.666666700;
      
      r = (uint32_t) degrees ; // cast and return (masking with F in proper location for this number)
    }
  
  // for negative case
  if(degrees <= 0)
    {
      degrees *= -1.0;  // flip to positive
      
      degrees *= 208.3;  // do transform
      degrees += 0.0;
      
      r = (uint32_t) degrees; // cast and return 
    }
  return r;
}


// Maps a ratio of zooming (1x 2x .. 12x) to the appropriate absolute value
// TODO: adapt this method so it's not a clone of the BRC300 version
uint32_t libviscaEVIHD1::zoom_lookup_table(int zoom)
{
  if(zoom >= 10) return 0x4000; // really zoom of 10.1x
  if(zoom >=  7) return 0x3800; // really zoom of 6.8x
  if(zoom >=  5) return 0x3000; // really zoom of 4.8x
  if(zoom >=  3) return 0x2800; // really zoom of 3.4x
  if(zoom >=  2) return 0x1800; // really zoom of 1.9x
  
  // assert: default case (zoom <= 1)
  return 0x0000;
}


// TODO: Find out proper values from geometric precision team
// TODO: adapt this method so it's not a clone of the BRC300 version
double libviscaEVIHD1::zoom_ratio_to_degreesTheta(int zoom) // convert zoom ratio to degrees field of view in horizontal (X or Theta) direction
{
  // Don't forget to use complete field of view, as opposed to result of arctan(0.5*pixels/focal length) -- that's only for 1/2 the triangle!
  if(zoom >= 10) return 0x4000; // really zoom of 10.1x
  if(zoom >=  7) return 0x3800; // really zoom of 6.8x
  if(zoom >=  5) return 0x3000; // really zoom of 4.8x
  if(zoom >=  3) return 0x2800; // really zoom of 3.4x
  if(zoom >=  2) return 0x1800; // really zoom of 1.9x
  
  // assert: default case (zoom <= 1)
  return 19.1 * 2; // real val 1/13
}

// TODO: Find out proper values from geometric precision team
// TODO: adapt this method so it's not a clone of the BRC300 version
double libviscaEVIHD1::zoom_ratio_to_degreesPhi(int zoom)   // convert zoom ratio to degrees field of view in vertical (Y or Phi) direction
{
  // Don't forget to use complete field of view, as opposed to result of arctan(0.5*pixels/focal length) -- that's only for 1/2 the triangle!
  if(zoom >= 10) return 0x4000; // really zoom of 10.1x
  if(zoom >=  7) return 0x3800; // really zoom of 6.8x
  if(zoom >=  5) return 0x3000; // really zoom of 4.8x
  if(zoom >=  3) return 0x2800; // really zoom of 3.4x
  if(zoom >=  2) return 0x1800; // really zoom of 1.9x
  
  // assert: default case (zoom <= 1)
  return 14.40 * 2; // real val 1/13
}




// set absolute pan/tilt position
VISCA_API uint32_t libviscaEVIHD1::set_pantilt_absolute_position(VISCAInterface_t *iface, VISCACamera_t *camera, uint32_t speed, uint32_t pan_position, uint32_t tilt_position)
{
  // Set up the packet we'll send
  VISCAPacket_t packet;
  _VISCA_init_packet(&packet);

  
  // Used proper codes for sending to camera
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x06);
  _VISCA_append_byte(&packet, 0x02);
  _VISCA_append_byte(&packet, 0x18 ); // pan rate of motion: 0x18 is fastest
  _VISCA_append_byte(&packet, 0x14 ); // tilt rate of motion: 0x14 is fastest

  
  // needed to have five bytes for pan, four bytes for tilt
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
VISCA_API uint32_t libviscaEVIHD1::set_zoom_absolute_position(VISCAInterface_t* iface, VISCACamera_t* camera, uint32_t zoom)
{
  // minimum and maximum zoom values for the EVI-HD1
  uint32_t min = 0x00000000;
  uint32_t max = 0x00004000;

  // clamp zoom to appropriate values
  if(zoom > max) zoom = max;
  if(zoom < min) zoom = min;
  
  // standard libvisca function works with the EVH-HD1
  return VISCA_set_zoom_value(iface, camera, zoom);
}


// TODO: adapt this method so it's not a clone of the BRC300 version
VISCA_API uint32_t libviscaEVIHD1::set_camera_power(VISCAInterface_t* iface, VISCACamera_t* camera, bool powerStatus)
{
  VISCAPacket_t packet;

  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x00);

  if(powerStatus)
    _VISCA_append_byte(&packet, 0x02); // power ON with byte 0x02
  else
    _VISCA_append_byte(&packet, 0x03); // power OFF with byte 0x03

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}


VISCA_API uint32_t libviscaEVIHD1::set_dzoom_off(VISCAInterface_t* iface, VISCACamera_t* camera)
{
  VISCAPacket_t packet;

  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x06);
  _VISCA_append_byte(&packet, 0x03);

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}

VISCA_API uint32_t libviscaEVIHD1::set_menu_off(VISCAInterface_t* iface, VISCACamera_t* camera)
{
  VISCAPacket_t packet;

  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x06);
  _VISCA_append_byte(&packet, 0x06);
  _VISCA_append_byte(&packet, 0x03);

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}

char libviscaEVIHD1::iris_lookup_table(double f) // pick an f-stop
{
  // from Sony technical manual, pg 35
  if( f <= 1.8 ) return 0x11;
  if( f <= 2.0 ) return 0x10;
  if( f <= 2.4 ) return 0x0F;
  if( f <= 2.8 ) return 0x0E;
  if( f <= 3.4 ) return 0x0D;
  if( f <= 4.0 ) return 0x0C;
  if( f <= 4.8 ) return 0x0B;
  if( f <= 5.6 ) return 0x0A;
  if( f <= 6.8 ) return 0x09;
  if( f <= 8.0 ) return 0x08;
  if( f <= 9.6 ) return 0x07;
  if( f <= 11  ) return 0x06;
  if( f <= 14  ) return 0x05;
  if( f <= 16  ) return 0x04;
  if( f <= 19  ) return 0x03;
  if( f <= 22  ) return 0x02;
  if( f <= 26  ) return 0x01;
  
  // assert: iris closed
  return 0x00;  
}

// manually set the shutter speed -- 0x00 = 1/2 sec, 0x06 = normal, 0x15 = fastest
VISCA_API uint32_t libviscaEVIHD1::set_absolute_shutter(VISCAInterface_t* iface, VISCACamera_t* camera, char value)
{
  // set end stops  
  if(value > 0x15) value = 0x15;
  
  VISCAPacket_t packet;

  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x0A); // set to A for shutter
  _VISCA_append_byte(&packet, 0x00);
  _VISCA_append_byte(&packet, 0x00);
  _VISCA_append_byte(&packet, ((value & 0xF0) >> 4) );
  _VISCA_append_byte(&packet, ((value & 0x0F)     ) );

  return _VISCA_send_packet_with_reply(iface, camera, &packet);  
}

// manually set the fstop -- use mapping method
VISCA_API uint32_t libviscaEVIHD1::set_absolute_iris(VISCAInterface_t* iface, VISCACamera_t* camera, char value)
{ 
  // set end stops  
  if(value > 0x11) value = 0x07;

  VISCAPacket_t packet;
  
  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x0B); // set to B for iris
  _VISCA_append_byte(&packet, 0x00);
  _VISCA_append_byte(&packet, 0x00);
  _VISCA_append_byte(&packet, ((value & 0xF0) >> 4) );
  _VISCA_append_byte(&packet, ((value & 0x0F)     ) );

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}

// manually set the gain -- 0x00 = -3db, 0x01 = normal, 0x07 = max
VISCA_API uint32_t libviscaEVIHD1::set_absolute_gain(VISCAInterface_t* iface, VISCACamera_t* camera, char value)
{
  // set end stops
  if(value > 0x07) value = 0x07;
  
  VISCAPacket_t packet;

  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x0C); // set to C for gain
  _VISCA_append_byte(&packet, 0x00);
  _VISCA_append_byte(&packet, 0x00);
  _VISCA_append_byte(&packet, ((value & 0xF0) >> 4) );
  _VISCA_append_byte(&packet, ((value & 0x0F)     ) );

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}


// automatic / manual exposure modes
VISCA_API uint32_t libviscaEVIHD1::set_auto_exposure(VISCAInterface_t* iface, VISCACamera_t* camera)
{
  VISCAPacket_t packet;
  
  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x39);
  _VISCA_append_byte(&packet, 0x00);

  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}

VISCA_API uint32_t libviscaEVIHD1::set_manual_exposure(VISCAInterface_t* iface, VISCACamera_t* camera)
{
  VISCAPacket_t packet;
  
  // set up header appropriately
  _VISCA_init_packet(&packet);
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x04);
  _VISCA_append_byte(&packet, 0x39);
  _VISCA_append_byte(&packet, 0x03);

  return _VISCA_send_packet_with_reply(iface, camera, &packet);  
}
