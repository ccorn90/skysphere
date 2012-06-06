//Trial Blackmagic program
//Ian McGinnis
//1/25/2012
//@St Olaf College

#include "DeckLinkAPI.h"
#include "main.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

// added CJC 1/26/2012
#include <fstream>

using namespace std;

pthread_mutex_t	sleepMutex;
pthread_cond_t	sleepCond;
int videoOutputFile = -1;
int audioOutputFile = -1;

IDeckLink *deckLink;
IDeckLinkInput	*deckLinkInput;
IDeckLinkDisplayModeIterator *displayModeIterator;

static BMDTimecodeFormat g_timecodeFormat = 0;
static int g_videoModeIndex = -1;
static int g_audioChannels = 2;
static int g_audioSampleDepth = 16;
const char * g_videoOutputFile = NULL;
const char * g_audioOutputFile = NULL;
static int g_maxFrames = -1;

static unsigned long frameCount = 0;

//define CaptureDelegate
DeckLinkCaptureDelegate::DeckLinkCaptureDelegate() : m_refCount(0)
{
	pthread_mutex_init(&m_mutex, NULL);
}

DeckLinkCaptureDelegate::~DeckLinkCaptureDelegate()
{
	pthread_mutex_destroy(&m_mutex);
}

ULONG DeckLinkCaptureDelegate::AddRef(void)
{
	pthread_mutex_lock(&m_mutex);
		m_refCount++;
	pthread_mutex_unlock(&m_mutex);

	return (ULONG)m_refCount;
}

ULONG DeckLinkCaptureDelegate::Release(void)
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
HRESULT DeckLinkCaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioFrame)
{
	IDeckLinkVideoFrame*	                rightEyeFrame = NULL;
	IDeckLinkVideoFrame3DExtensions*        threeDExtensions = NULL;
	void*					frameBytes;
	void*					audioFrameBytes;
	
	// Handle Video Frame
	if(videoFrame)
	{	
	  //removed 3D interface

		if (videoFrame->GetFlags() & bmdFrameHasNoInputSource)
		{
			fprintf(stderr, "Frame received (#%lu) - No input signal detected\n", frameCount);
		}
		else
		{
			const char *timecodeString = NULL;
			if (g_timecodeFormat != 0)
			{
				IDeckLinkTimecode *timecode;
				if (videoFrame->GetTimecode(g_timecodeFormat, &timecode) == S_OK)
				{
					timecode->GetString(&timecodeString);
				}
			}

			fprintf(stderr, "Frame received (#%lu) [%s] - %s - Size: %li bytes\n", 
				frameCount,
				timecodeString != NULL ? timecodeString : "No timecode",
			        "Valid Frame", 
				videoFrame->GetRowBytes() * videoFrame->GetHeight()
				);

			if (timecodeString)
				free((void*)timecodeString);

			//this is where the frame is written to a file!
			if (videoOutputFile != -1)
			{
			  
			  // do it with an fstream object, for kicks
			  fstream of ("/home/mcginnis/atp12-skysphere/image_devel/bkmagic_out.ppm", fstream::out);
			  
			  //getRowBytes needs to be divided by 3
			  char fileHeader[100];
			  sprintf(fileHeader, "P6\n%li %li\n255\n",videoFrame->GetWidth(),videoFrame->GetHeight());
			  cerr << "width/height " << videoFrame->GetWidth() << " " << videoFrame->GetHeight()<<endl;
			  cerr << "pixel format: " << videoFrame->GetPixelFormat() << endl;
			  cerr << "Pixel width/RowBytes "<< videoFrame->GetWidth() << " " << videoFrame->GetRowBytes()<<endl;

			  // try writing to an fstream object
			  of << fileHeader;
			  
			  videoFrame->GetBytes(&frameBytes);

			  write(videoOutputFile, fileHeader, strlen(fileHeader)); 
			  write(videoOutputFile, frameBytes, videoFrame->GetRowBytes() * videoFrame->GetHeight());

			  // write bytes to fstream object
			  of.write((char*)frameBytes, videoFrame->GetRowBytes() * videoFrame->GetHeight());
			  of.close();

			  cerr << "Wrote to file! " << videoOutputFile << endl;
			}
		}
		
		frameCount++;

		if (g_maxFrames > 0 && frameCount >= g_maxFrames)
		{
		  pthread_cond_signal(&sleepCond);
		}
	}

	// Handle Audio Frame - our caring about this is minimal.
	if (audioFrame)
	{	}
    return S_OK;
}

HRESULT DeckLinkCaptureDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}


int main(int argc, char ** argv)
{
  IDeckLinkIterator* deckLinkIterator;
  DeckLinkCaptureDelegate* delegate; //needs a constructor defined? This could potentially be used to capture things - user defined
  IDeckLinkDisplayMode *displayMode;
  BMDVideoInputFlags inputFlags = 0;
  BMDDisplayMode selectedDisplayMode= bmdModeNTSC; // really ought to be a different setting, but cannot get changes here to make any difference in what happens on the output.   bmdModeHD1080i5994
  BMDPixelFormat pixelFormat = bmdFormat8BitYUV;
  int	displayModeCount = 0;
  int	exitStatus = 1;
  int	ch;
  bool 	foundDisplayMode = false;
  HRESULT result; 

  pthread_mutex_init(&sleepMutex, NULL);
  pthread_cond_init(&sleepCond, NULL);

  // Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
  deckLinkIterator = CreateDeckLinkIteratorInstance();
  if (deckLinkIterator == NULL)
    {
      fprintf(stderr, "A DeckLink iterator could not be created.  The DeckLink drivers may not be installed.\n");
      return 1; //goto bail; //??
    }

  //Connect to the first Decklink Card we find
  result = deckLinkIterator->Next(&deckLink);
  if (result !=S_OK)
    {
      fprintf(stderr, "No DeckLink PCI cards found.\n");
      return 2;
    }

  if (deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput) != S_OK)
    {return 3;}
  
  delegate = new DeckLinkCaptureDelegate();
  deckLinkInput->SetCallback(delegate);
  
  // Obtain an IDeckLinkDisplayModeIterator to see if the display mode is supported on output
  result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
  if (result != S_OK)
    {
      fprintf(stderr, "Could not obtain the video output display mode iterator - result = %08x\n", result);
      goto bail;
    }
  ////////////////////////////////////////////SAVE AND CAPTURE SETTINGS///////////////////////////////////////////////////////

  g_videoOutputFile = "/home/mcginnis/atp12-skysphere/image_devel/test.ppm"; //not sure what format it'll be in.
  g_maxFrames = 1; //shouldn't need this - unless I specified it at 1 - this tells the code to grab one frame
  g_videoModeIndex = 6; //want mode 6 for 1080i @ 29.97fps for blackmagic cards. Found by running Capture in the SDK
  //Trial and error: modes 6 and 9 work with the Sony EVI-HD1's
  //THought for capturing - maxframes specifies # of pics taken. Then just increment file name and include pertinant .ppm info

  /* //g_videoModeIndex settings: (from running Capture.cpp - available with our Decklink Duo card. IM - 12/26/2012)
    0:  NTSC                 	 720 x 486 	 29.97 FPS
    1:  NTSC 23.98           	 720 x 486 	 23.976 FPS
    2:  PAL                  	 720 x 576 	 25 FPS
    3:  HD 1080p 23.98       	 1920 x 1080 	 23.976 FPS
    4:  HD 1080p 24          	 1920 x 1080 	 24 FPS
    5:  HD 1080p 25          	 1920 x 1080 	 25 FPS
    6:  HD 1080p 29.97       	 1920 x 1080 	 29.97 FPS
    7:  HD 1080p 30          	 1920 x 1080 	 30 FPS
    8:  HD 1080i 50          	 1920 x 1080 	 25 FPS
    9:  HD 1080i 59.94       	 1920 x 1080 	 29.97 FPS
    10:  HD 1080i 60          	 1920 x 1080 	 30 FPS
    11:  HD 720p 50           	 1280 x 720 	 50 FPS
    12:  HD 720p 59.94        	 1280 x 720 	 59.9401 FPS
    13:  HD 720p 60           	 1280 x 720 	 60 FPS
  */
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  

  // opens the file we're going to write to
  if (g_videoOutputFile != NULL)
    {
      videoOutputFile = open(g_videoOutputFile, O_WRONLY|O_CREAT|O_TRUNC, 0664);
      if (videoOutputFile < 0)
	{
	  fprintf(stderr, "Could not open video output file \"%s\"\n", g_videoOutputFile);
	  goto bail;
	}
    }
  
  //check our display mode choice for validity
  while (displayModeIterator->Next(&displayMode) == S_OK)
    {
      if (g_videoModeIndex == displayModeCount)
	{
	  BMDDisplayModeSupport result;
	  const char *displayModeName;
	  
	  foundDisplayMode = true;
	  displayMode->GetName(&displayModeName);
	  selectedDisplayMode = displayMode->GetDisplayMode();
	  
	  deckLinkInput->DoesSupportVideoMode(selectedDisplayMode, pixelFormat, bmdVideoInputFlagDefault, &result, NULL);
	  
	  if (result == bmdDisplayModeNotSupported)
	    {
	      fprintf(stderr, "The display mode %s is not supported with the selected pixel format\n", displayModeName);
	      goto bail;
	    }
       
	  break;
	}
      displayModeCount++;
      displayMode->Release();
    }

  if (!foundDisplayMode)
    {
      fprintf(stderr, "Invalid mode %d specified\n", g_videoModeIndex);
      goto bail;
    }

  result = deckLinkInput->EnableVideoInput(selectedDisplayMode, pixelFormat, inputFlags);
  if(result != S_OK)
    {
      fprintf(stderr, "Failed to enable video input. Is another application using the card?\n");
      goto bail;
    }
  result = deckLinkInput->StartStreams();
  if(result != S_OK)
    {
      goto bail;
    }
  
    // All Okay.
    exitStatus = 0;
    
    // Block main thread until signal occurs
    pthread_mutex_lock(&sleepMutex);
    pthread_cond_wait(&sleepCond, &sleepMutex);
    pthread_mutex_unlock(&sleepMutex);
    fprintf(stderr, "Stopping Capture\n");
    
 bail:
   	
    if (videoOutputFile)
      close(videoOutputFile);
     
    if (displayModeIterator != NULL)
      {
	displayModeIterator->Release();
	displayModeIterator = NULL;
      }
    
    if (deckLinkInput != NULL)
      {
        deckLinkInput->Release();
        deckLinkInput = NULL;
      }
    
    if (deckLink != NULL)
      {
        deckLink->Release();
        deckLink = NULL;
      }
    
    if (deckLinkIterator != NULL)
      deckLinkIterator->Release();
    
    return exitStatus;
}
