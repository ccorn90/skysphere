// ImgCap_Blackmagic.hpp
// @author Ian McGinnis, Chris Cornelius, Charles Nye
// Created 01/24/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Uses the Blackmagic DeckLink card video API, which is open-source software.  blackmagic.com
// Much of this class is based on the Capture executable in the Blackmagic SDK Linux samples folder.

#ifndef _IMGCAP_BLACKMAGIC_HPP_
#define _IMGCAP_BLACKMAGIC_HPP_

#include "ImgCap_base.hpp"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
using std::cerr; using std::endl; using std::fstream;
#include "DeckLinkAPI.h" //need to test to make sure this is the correct #include

class ImgCap_Blackmagic;

// object which handles callbacks from the decklink interface (such as getting a new frame)
class BlackmagicCallbackObj : public IDeckLinkInputCallback
{
  pthread_cond_t* sleeperCondToCall;
  ImgCap_Blackmagic* capObj;
public:
  
  string fileNameToSave;
  
  BlackmagicCallbackObj(pthread_cond_t* sleeper, ImgCap_Blackmagic* icobj) :  m_refCount(0)
  {
    sleeperCondToCall = sleeper;
    capObj = icobj;
    pthread_mutex_init(&m_mutex, NULL);
  }
  ~BlackmagicCallbackObj()
  {
    pthread_mutex_destroy(&m_mutex);
  }

  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
  virtual ULONG STDMETHODCALLTYPE AddRef(void);
  virtual ULONG STDMETHODCALLTYPE  Release(void);
  virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
  virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);
  
private:
  ULONG	m_refCount;
  pthread_mutex_t m_mutex;
};





class ImgCap_Blackmagic : public ImgCap_base
{
  friend class BlackmagicCallbackObj;
  
protected:
  BlackmagicCallbackObj* callbackObj; // object which handles the callbacks from the blackmagic card

  // to handle thread sleeping
  pthread_mutex_t sleepMutex;
  pthread_cond_t sleepCond;

  // the actual deckLink objects we'll use
  IDeckLink* interface; 
  IDeckLinkInput* input_interface;

  // other data used to configure the blackmagic capture
  BMDDisplayMode selectedDisplayMode; // defines how we're capturing things from the card - initialized in openConnection()
  BMDPixelFormat pixelFormat;
  
  // interior methods
  void cleanUp();  // deallocates pointers, etc
  void signalPthread() {
    pthread_cond_signal(&sleepCond);
  }
  
public:
  // create an ImgCap object with a devicename, etc
  ImgCap_Blackmagic(const char* device, int deviceIndex) : ImgCap_base(device, deviceIndex, ImgCap_base::BLACKMAGIC) 
  {
    interface = NULL;
    input_interface = NULL;
    pixelFormat = bmdFormat8BitYUV;

    // set up the callbackObj with proper information
    callbackObj = new BlackmagicCallbackObj(&sleepCond, this);
  }

  
  ImgCap_Blackmagic(const char* device); // create an ImgCap object with the first constructor,
  ~ImgCap_Blackmagic() // destruct the ImgCap_V4L2 object
  {
    if(callbackObj) delete callbackObj;
  }
  // Methods overridden from ImgCap_base
  virtual void grabImageToFile(); //saves images with fileNumber, camera, and time
  virtual int grabImageToPPM(const char * fileName); //save an image w/given filename
  virtual bool openConnection(); // prepare to receive Image structs from the device
  virtual bool closeConnection(); // elegantly break connection to the device
  //virtual Image grabImage(); 
  //virtual bool good();
};


#endif
