// CamCtrl_base.hpp
// @author Chris Cornelius
// Created 01/22/2012
// ATP 2012, Skysphere Project St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Declaration of CamCtrl base class, for controlling a Sony camera via the VISCA interface.
// Overrides CameraController class from previous editions

#ifndef _CAM_CTRL_HPP_
#define _CAM_CTRL_HPP_

#include "libvisca.h"      // for the VISCA camera-control serial interface
#include <fcntl.h>
#include <string>
#include <ostream>
using std::string;


class CamCtrl_base {
public:
  // the different types possible for this CamCtrl object
  enum CtrlType {
    ERROR = -1,
    BRC300 = 1,
    EVIHD1 = 2
  };

protected:
  string _serialDevice;  // the device of the serial port we'll use to send VISCA commands to the camera.
  int _viscaID;     // address of DIP switch on bottom of camera - will be 1 to 7 and must be unique on this serial link.
  
  // Used for VISCA communications and storage of key camera info
  bool _open;  // is the interface open?
  VISCAInterface_t _iface;
  VISCACamera_t _vcamera;
  
  // what type is this?
  CtrlType _type;
  
public:
  // Constructors, destructor
  CamCtrl_base(void) {
    _serialDevice = "";
    _viscaID = 1;
    _open = false;
    _type = CamCtrl_base::ERROR;
  }
  CamCtrl_base(CamCtrl_base::CtrlType t) {
    _serialDevice = "";
    _viscaID = 1;
    _open = false;
    _type = t;
  }
  CamCtrl_base(string serialDevice, int viscaID, CamCtrl_base::CtrlType t) {
    _serialDevice = serialDevice;
    _viscaID = viscaID;
    _open = false;   
    _type = t;
  }
  ~CamCtrl_base(void) {
    closeSerial();
  }
  
  // Accessors for object variables
  bool isopen() { return _open; }
  string serialDevice() { return _serialDevice; }
  int viscaID() { return _viscaID; }
  VISCACamera_t vcamera() { return _vcamera; }
  CamCtrl_base::CtrlType type() { return _type; }

  // Camera-specific methods
  // Set absolute values for zoom, focus, pan, tilt - these methods must be overridden!
  virtual bool setPan(double theta) { }
  virtual bool setTilt(double phi) { }
  virtual bool setPanTilt(double theta, double phi) { }
  virtual bool setZoom(uint32_t absolute) { }  // use a direct indexing value
  virtual bool setZoom(int ratio) { }  // choose a zoom ratio: 1x 2x 3x ... 12x
  // TODO: setFocus(uint32_t absolute);  // use a direct indexing value
  // TODO: setFocus(int depth);  // choose a focal distace in centimeters: inf=10000 500 200 120 80 50 20 11 6c 2c 1
  
  // Power camera on and off
  virtual bool setPowerOn() { }
  virtual bool setPowerOff() { }
  
  // Set absolute values for exposure
  virtual bool setExposureAuto(bool a) { }
  virtual bool setDzoomOff() { }
  virtual bool setShutter(char value ) { } // 0x02 = 1/3 sec, 0x06 = normal, 0x15 = fastest
  virtual bool setIris(double fstop) { }
  virtual bool setGain(char value) { } // 0x00 = -3db, 0x01 = normal, 0x07 = max
  
  // TODO: other stuffs

  // Modifiers for object variables - be careful about what you modify!
  void set_serialDevice(string sd) { _serialDevice = sd; }
  void set_viscaID(int id) { _viscaID = id; }

  // Serial connection management methods
  virtual bool openSerial() {
    // try to open the serial port
    if ( VISCA_open_serial(&_iface, _serialDevice.c_str() ) != VISCA_SUCCESS ) {
      return _open = false;
    }
    // assert: serial port opened okay
    _iface.broadcast = 0;
    if( VISCA_set_address(&_iface, &_viscaID) != VISCA_SUCCESS) {
      return _open = false;
    }
    _vcamera.address = _viscaID;

    // assert: initialized okay
    return _open = true;
  }
  virtual bool initializeCamera() {
    // load camera details into _vcamera, so we have them
    if( VISCA_get_camera_info(&_iface, &_vcamera) != VISCA_SUCCESS )
      return false;
    // assert: loaded okay
    return true;
  }
  virtual bool closeSerial() {
    // if connection isn't open, automatically succeed
    if(!_open) return true;
    // required VISCA connection closing
    if (VISCA_close_serial(&_iface) != VISCA_SUCCESS) return false;
    // assert: closed connection okay
    return true;
  }  

  friend std::ostream& operator<<(std::ostream& os, CamCtrl_base::CtrlType t) {
    if(t == CamCtrl_base::BRC300) os << "BRC300";
    else if (t == CamCtrl_base::EVIHD1) os << "EVIHD1";
    return os;
  }
};

#endif // _CAM_CTRL_HPP_
