// CameraController.hpp
// @author Chris Cornelius
// Created 01/05/2012
// Skysphere project, ATP 2012
// St. Olaf College
// Declaration of CameraController class, for controlling a Sony camera via the VISCA interface

#ifndef _CAMERA_CONTROLLER_HPP_
#define _CAMERA_CONTROLLER_HPP_

#include "libvisca.h"      // for the VISCA camera-control serial interface
#include "libviscaBRC300.hpp" // for the namespace to control the BRC-300 camera
#include <fcntl.h>
#include <errno.h>
#include <string>
using std::string;


class CameraController {
private:
  string _serialDevice;  // the device of the serial port we'll use to send VISCA commands to the camera.
  int _viscaID;     // address of DIP switch on bottom of camera - will be 1 to 7 and must be unique on this serial link.
  
  // Used for VISCA communications and storage of key camera info
  bool _open;  // is the interface open?
  VISCAInterface_t _iface;
  VISCACamera_t _vcamera;

  
public:
  // Constructors, destructor
  CameraController(void);
  CameraController(string serialDevice, int viscaID);
  ~CameraController(void);
  
  // Accessors for object variables
  bool isopen() { return _open; }
  string serialDevice() { return _serialDevice; }
  int viscaID() { return _viscaID; }
  VISCACamera_t vcamera() { return _vcamera; }

  // Modifiers for object variables - be careful about what you modify!
  void set_serialDevice(string sd) { _serialDevice = sd; }
  void set_viscaID(int id) { _viscaID = id; }

  // Serial connection management methods
  bool openSerial();
  bool initializeCamera();
  bool closeSerial();
  
  // Set absolute values for zoom, focus, pan, tilt
  bool setPan(double theta);
  bool setTilt(double phi);
  bool setPanTilt(double theta, double phi);
  bool setZoom(uint32_t absolute); // use a direct indexing value
  bool setZoom(int ratio);  // choose a zoom ratio: 1x 2x 3x ... 12x
  // TODO: setFocus(uint32_t absolute);  // use a direct indexing value
  // TODO: setFocus(int depth);  // choose a focal distace in centimeters: inf=10000 500 200 120 80 50 20 11 6c 2c 1
  

  // Power camera on and off
  // TODO: determine if these work, or need to be rewritten
  bool setPowerOn();
  bool setPowerOff();
  
  // TODO: other stuffs
  
};

#endif // _CAMERA_CONTROLLER_HPP_
