// REVISIONS: For skysphere project in ATP class January 2011, St. Olaf College
//            by Chris Cornelius


#include <iostream> // ADDED 1/5/12
using std::cout;
using std::endl;

#include "libvisca.h"  // CHANGED 1/4/12
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h> /* File control definitions */
#include <errno.h> /* Error number definitions */

#define EVI_D30


// rewritten to represent camera pan position for the BRC-300 camera.  We'll need to write a VISCA library for this, I guess.
VISCA_API uint32_t
VISCA_set_pantilt_absolute_position_special(VISCAInterface_t *iface, VISCACamera_t *camera, uint32_t pan_speed, uint32_t tilt_speed, int pan_position, int tilt_position)
{
  VISCAPacket_t packet;

  uint32_t pan_pos=(uint32_t) pan_position;
  uint32_t tilt_pos=(uint32_t) tilt_position;

  _VISCA_init_packet(&packet);

  // Made sure to look up the appropriate codes for sending a pan/tilt absolute position to the camera
  _VISCA_append_byte(&packet, VISCA_COMMAND);
  _VISCA_append_byte(&packet, 0x06);
  _VISCA_append_byte(&packet, 0x02);
  _VISCA_append_byte(&packet, 0x18);
  _VISCA_append_byte(&packet, 0x00);

  
  // needed to have five bytes for pan, four bytes for tilt
  _VISCA_append_byte(&packet, (pan_pos & 0xf0000) >> 16);
  _VISCA_append_byte(&packet, (pan_pos & 0x0f000) >> 12);
  _VISCA_append_byte(&packet, (pan_pos & 0x00f00) >>  8);
  _VISCA_append_byte(&packet, (pan_pos & 0x000f0) >>  4);
  _VISCA_append_byte(&packet,  pan_pos & 0x0000f       );

  _VISCA_append_byte(&packet, (tilt_pos & 0xf000) >> 12);
  _VISCA_append_byte(&packet, (tilt_pos & 0x0f00) >> 8);
  _VISCA_append_byte(&packet, (tilt_pos & 0x00f0) >> 4);
  _VISCA_append_byte(&packet, tilt_pos & 0x000f);
 

  //_VISCA_append_byte(&packet, 0xFF); // it is NOT REQUIRED to add the terminating 0xff byte


  return _VISCA_send_packet_with_reply(iface, camera, &packet);
}



int main(int argc, char **argv)
{

  VISCAInterface_t iface;
  VISCACamera_t camera;


  int camera_num;
  uint8_t value;
  uint16_t zoom;
  int pan_pos, tilt_pos;
  bool cont_var;

  if (argc<2)
    {
      fprintf(stderr,"%s usage: %s <serial port device>\n",argv[0],argv[0]);
      exit(1);
    }

  if (VISCA_open_serial(&iface, argv[1])!=VISCA_SUCCESS)
    {
      fprintf(stderr,"%s: unable to open serial device %s\n",argv[0],argv[1]);
      exit(1);
    }
  // ADDED 1/5/12
  else {
      fprintf(stderr,"%s: Successfully opened serial device %s\n",argv[0],argv[1]);  
  }

  camera_num = 1;


  iface.broadcast=0;
  VISCA_set_address(&iface, &camera_num);
  camera.address=1;
  fprintf(stderr, "Clearing: %i \n", VISCA_clear(&iface, &camera) );

  VISCA_get_camera_info(&iface, &camera);
  fprintf(stderr,"Some camera info:\n------------------\n");
  fprintf(stderr,"vendor: 0x%04x\n model: 0x%04x\n ROM version: 0x%04x\n socket number: 0x%02x\n",
	  camera.vendor, camera.model, camera.rom_version, camera.socket_num);


  // Do a couple of checks and initialization movements
  if (VISCA_get_power(&iface, &camera, &value)!=VISCA_SUCCESS)
    fprintf(stderr,"error getting power\n");
  else
    fprintf(stderr,"power status: 0x%02x\n",value);

  if (VISCA_set_pantilt_absolute_position_special(&iface, &camera,5,5,0x00000,0x0000)!=VISCA_SUCCESS)
    fprintf(stderr,"Error setting pan tilt absolute position.\n");
  else
    fprintf(stderr,"Set pan tilt absolute position.\n");

  if (VISCA_get_pantilt_position(&iface, &camera, &pan_pos, &tilt_pos)!=VISCA_SUCCESS)
    fprintf(stderr,"error getting pan tilt absolute position\n");
  else
    fprintf(stderr,"Absolute position, Pan value: %x, Tilt value: %x\n",pan_pos,tilt_pos);




  // a simple interface which lets you move the camera multiple times.
  cont_var = true;
  uint32_t panvar = 0x00000;  // holds the value of the pan/tilt
  while (cont_var) {
    
    fprintf(stderr,"Enter a pan position: 0x");
    fscanf(stdin,"%x",&panvar);

    // to break out
    if (panvar == 0xFFFFFF)
      {
	cont_var = false;
	continue;
      }
    
    if (VISCA_set_pantilt_absolute_position_special(&iface, &camera,5,5,panvar,0x0000)!=VISCA_SUCCESS)
      fprintf(stderr,"Error setting pan tilt absolute position.\n");
    else
      fprintf(stderr,"Set pan tilt absolute position.\n");
    
    
    if (VISCA_get_pantilt_position(&iface, &camera, &pan_pos, &tilt_pos)!=VISCA_SUCCESS)
      fprintf(stderr,"Error getting pan tilt absolute position\n");
    else
      fprintf(stderr,"Absolute position, Pan value: 0x%8x, Tilt value: 0x%8x\n",pan_pos,tilt_pos);

  }


  

  // clean up by reading any extra data off the serial port.  (should be empty).
  fprintf(stderr,"Cleaning up serial connection...\n");
  {
    unsigned char packet[3000];
    uint32_t buffer_size = 3000;
    if (VISCA_unread_bytes(&iface, packet, &buffer_size)!=VISCA_SUCCESS)
    {
      uint32_t i;
      fprintf(stderr, "ERROR: %u bytes not processed", buffer_size);
      for (i=0;i<buffer_size;i++)
        fprintf(stderr,"%2x ",packet[i]);
      fprintf(stderr,"\n");
    }
  }
  VISCA_close_serial(&iface);
  fprintf(stderr,"Closed serial connection\n");
  exit(0);


}
