// ImgCap_Blackmagic.cpp
// @author Ian McGinnis, Chris Cornelius, Charles Nye
// Created 01/24/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

#include "ImgCap_Blackmagic.hpp"

////////////////////
// for the BlackmagicCallbackObj class -- basically copied from the Capture example in the Blackmagic SDK


ULONG  BlackmagicCallbackObj::AddRef(void)
{
  pthread_mutex_lock(&m_mutex);
  m_refCount++;
  pthread_mutex_unlock(&m_mutex);
  
  return (ULONG)m_refCount;
}

ULONG  BlackmagicCallbackObj::Release(void)
{
  pthread_mutex_lock(&m_mutex);
  m_refCount--;
  pthread_mutex_unlock(&m_mutex);
  
  if (m_refCount == 0)
    {
      delete this;
      return 0;
    }
  
  return (ULONG)m_refCount;
}

HRESULT  BlackmagicCallbackObj::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioFrame)
{
  void* frameBytes;
  
  // Handle Video Frame
  if(videoFrame)
    {	
      if (videoFrame->GetFlags() & bmdFrameHasNoInputSource)
	{
	  fprintf(stderr, "Frame received - No input signal detected\n");
	}
      else
	{  
	  cerr << "Got a frame, has video." << endl;
	  
	  // time to write to a file!
	  // do it with an fstream object, for kicks
	  fstream of (fileNameToSave.c_str(), fstream::out);
	  
	  //getRowBytes needs to be divided by 3
	  char fileHeader[100];
	  sprintf(fileHeader, "P6\n%li %li\n255\n",videoFrame->GetWidth(),videoFrame->GetHeight());
	  
	  cerr << "Preparing to write file: " << fileHeader << endl;
	  
	  // try writing to an fstream object
	  of << fileHeader;
	  
	  // get the bytes we need
	  videoFrame->GetBytes(&frameBytes);
	  
	  // write bytes to fstream object
	  of.write(fileHeader, strlen(fileHeader)); 
	  of.write((char*)frameBytes, videoFrame->GetRowBytes() * videoFrame->GetHeight());
	  
	  of.close();
	  
	  cerr << "Wrote to file! " << fileNameToSave << endl;
	}
    }
  
  
  // we only ever want to have one frame, so we'll call the signal to wake up the other thread
  // pthread_cond_signal(sleeperCondToCall);  // for some reason, this function doesn't return.  So trying something else.
  // try changing to pthread_cond_broadcase(sleeperCondToCall);
  capObj->signalPthread();

  // return to caller
  return S_OK;
}

 

HRESULT  BlackmagicCallbackObj::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

////////////////////

bool ImgCap_Blackmagic::openConnection(){
  // get the capturing object we want (using an IDeckLinkIterator and comparing with dev_name)
  
  // Create an IDeckLinkIterator
  IDeckLinkIterator* iterator = CreateDeckLinkIteratorInstance();
  if( !iterator ) {
    cleanUp();
    return false;
  }
  
  // assert: iterator is functional and ready to search the Blackmagic devices for the one we want

  // Enumerate all DeckLink cards in the system, comparing to dev_name to decide which we want
  while (iterator->Next(&interface) == S_OK)
    { 
      if(interface == NULL) // didn't find an interface - bail so that the null catch returns false
	break;
      
      // assert: the iterator found an interface
      
      // TODO: check to see if this interface is the one we want
      // if( deckLink->GetModelName((const char **) &deviceNameString) == S_OK) ... etc

      // WORKAROUND 1/26/12 - just return the first one we find
      break;      
    }
  
  if(interface == NULL) // found no interface that's valid
    {
      if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Could not find any Blackmagic devices." << endl;
      cleanUp();
      return false;
    }

  // assert: interface is a valid interface for this connection

  // check in with the interface object and load input_interface for this interface
  if(interface->QueryInterface(IID_IDeckLinkInput, (void**)&input_interface) != S_OK)
    {
       if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Fatal - could not connect to the interface selected." << endl;
      cleanUp();
      return false;
    }
  
 if(input_interface == NULL) // found no interface that's valid
    {
      if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Null input_interface!" << endl;
      cleanUp();
      return false;
    }

  // assert: input_interface is ready to use
  
  // check to make sure the card supports our chosed DisplayMode
  IDeckLinkDisplayModeIterator* displayModeIterator = NULL;
  IDeckLinkDisplayMode* displayMode;  
  int displayModeCount = 0;
  bool foundDisplayMode = false;
  int g_videoModeIndex = device_input_index;  // the video mode we'll be wanting
  if(input_interface->GetDisplayModeIterator(&displayModeIterator) != S_OK)
    {
      if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Could not obtain a DisplayModeIterator." << endl;
      cleanUp();
      return false;
    }

  while (displayModeIterator->Next(&displayMode) == S_OK)
    {
      if (g_videoModeIndex == displayModeCount)
	{
	  BMDDisplayModeSupport result;
	  const char *displayModeName;
	  
	  foundDisplayMode = true;
	  displayMode->GetName(&displayModeName);
	  selectedDisplayMode = displayMode->GetDisplayMode();
	  
	  input_interface->DoesSupportVideoMode(selectedDisplayMode, pixelFormat, bmdVideoInputFlagDefault, &result, NULL);
	  
	  if (result == bmdDisplayModeNotSupported)
	    {
	      if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: card does not support given DisplayMode" << endl;
	      cleanUp();
	      return false;
	    }
       
	  break;
	}
      displayModeCount++;
      displayMode->Release();
    }
  
  if( !foundDisplayMode ) {
    if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Could not find given DisplayMode." << endl;
    cleanUp();
    return false;
  }

  // assert: we're okay to use the card with this DisplayMode... okay to free iterator
  displayModeIterator->Release();
  displayModeIterator = NULL;
  
  // set up the callback object we'll be using
  input_interface->SetCallback(callbackObj);
 
  // enableVideoInput
  if( input_interface->EnableVideoInput(selectedDisplayMode, pixelFormat, 0) != S_OK)
    {
      if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Could not enable video input" << endl;
      cleanUp();
      return false;
    }

  // assert: ready to return true
  return true;
}

bool ImgCap_Blackmagic::closeConnection(){
  // deallocate all
  cleanUp();
  return true;
}

void ImgCap_Blackmagic::grabImageToFile(){
  // check for null pointers
  if( input_interface == NULL || interface == NULL )
    {
      return;
    }
  
  
  // start the streams
  if( input_interface->StartStreams() != S_OK)
    {
      if(verboseErrorPrinting) cerr << "ImgCap_Blackmagic: Unable to start streaming!" << endl;
      return;
    }
 
  // configure callback object's filename
  callbackObj->fileNameToSave = "blackmagicTest.ppm";  
  
  // hand off to callback object by going to sleep
  pthread_mutex_lock(&sleepMutex); // Block main thread until signal occurs
  pthread_cond_wait(&sleepCond, &sleepMutex);  // waits
  pthread_mutex_unlock(&sleepMutex);
  
  // okay, so we returned from sleep.  now we can stop the stream and be done
  // TODO: figure out how to stop the stream
  // TODO: exit well
}

int ImgCap_Blackmagic::grabImageToPPM(const char * fileName){

}


void ImgCap_Blackmagic::cleanUp()
{
  if(interface) interface->Release();
  if(input_interface) input_interface->Release();  
}
