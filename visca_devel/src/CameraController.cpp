// CameraController.cpp
// @author Chris Cornelius
// Created 01/05/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Holds definitions for CameraController class methods

#include "CameraController.hpp"

#include <iostream>
using std::cerr;
using std::endl;

// constructors, destructor
CameraController::CameraController(void)
{
  _serialDevice = "";
  _viscaID = 1;
  _open = false;
}

CameraController::CameraController(string serialDevice, int viscaID)
{
  _serialDevice = serialDevice;
  _viscaID = viscaID;
  _open = false;
}

CameraController::~CameraController(void)
{
  // be sure to close the serial connection if it's not already
  closeSerial();

  // TODO: any other clean up?
}


bool CameraController::openSerial()
{
  // try to open the serial port
  if ( VISCA_open_serial(&_iface, _serialDevice.c_str() ) != VISCA_SUCCESS )
    {
      // failure case
      _open = false;
      return false;
    }
  
  // assert: successfully opened serial connection
  
  // set up the address of the camera we'll be talking to through this interface
  _iface.broadcast=0;
  if( VISCA_set_address(&_iface, &_viscaID) != VISCA_SUCCESS )
    { _open = false; return false; }

  _vcamera.address = _viscaID;
  
  // clear the serial port - TODO: make this work with VISCA_SUCCESS?
  VISCA_clear(&_iface, &_vcamera);
  
  // assert: everything opened okay
  _open = true;
  return true;
}

bool CameraController::initializeCamera()
{
  // send camera ON command
  
  // load camera details into _vcamera, so we have them
  if( VISCA_get_camera_info(&_iface, &_vcamera) != VISCA_SUCCESS )
    return false;

  fprintf(stderr,"Some camera info:\n------------------\n");
  fprintf(stderr,"vendor: 0x%04x\n model: 0x%04x\n ROM version: 0x%04x\n socket number: 0x%02x\n",
	  _vcamera.vendor, _vcamera.model, _vcamera.rom_version, _vcamera.socket_num);
  // have camera seek (0,0)
  // set default zoom settings, etc

  return true;
}

bool CameraController::closeSerial()
{
  // if connection isn't open, automatically succeed
  if(!_open) return true;

  // required VISCA connection closing
  if (VISCA_close_serial(&_iface) != VISCA_SUCCESS) return false;
  
  // assert: closed connection okay
  return true;
  
}


bool CameraController::setPanTilt(double theta, double phi)
{
  // convert theta and phi into camera range numbers
  uint32_t pan  = libviscaBRC300::degrees_to_uint(theta);
  uint32_t tilt = libviscaBRC300::degrees_to_uint(phi);
  
  uint32_t speed = 0x00000018; // maximum speed, captain.
  
  // clamp to acceptable range for camera's pan and tilt
  // TODO: rethink mapping functions (above) for better processing of end values
  if(pan > 0x08a58  && pan <= 0xf75a8) pan = 0xf75a8;
  if(pan > 0xffffff) pan = 0xffffff;
  if(tilt > 0x493d && tilt < 0xe796) tilt = 0xe796;

  // send command and wait for reply
  if( libviscaBRC300::set_pantilt_absolute_position(&_iface, &_vcamera, speed, pan, tilt)
      != VISCA_SUCCESS )
    return false;
  else
    return true;
}


bool CameraController::setZoom(uint32_t absolute) // for use with direct indexing value
{
  if( libviscaBRC300::set_zoom_absolute_position(&_iface, &_vcamera, absolute) != VISCA_SUCCESS )
    return false;
  else
    return true;
}



bool CameraController::setZoom(int ratio)  // choose a zoom ratio: 1x 2x 3x ... 12x
{
  // use lookup table for the appropriate camera
  uint32_t absolute = libviscaBRC300::zoom_lookup_table(ratio); 
  
  // use the appropriate BRC300 method
  if( libviscaBRC300::set_zoom_absolute_position(&_iface, &_vcamera, absolute) != VISCA_SUCCESS )
    return false;
  else
    return true;
}


bool CameraController::setPowerOn()
{
  // use the appropriate BRC300 method
  if( libviscaBRC300::set_camera_power(&_iface, &_vcamera, true) != VISCA_SUCCESS)
    return false;
  else
    return true;
}

bool CameraController::setPowerOff()
{
  // use the appropriate BRC300 method
  if( libviscaBRC300::set_camera_power(&_iface, &_vcamera, false) != VISCA_SUCCESS)
    return false;
  else
    return true;
}
