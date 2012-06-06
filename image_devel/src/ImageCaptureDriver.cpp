// ImageCaptureDriver.cpp
// Driver for ImageCapture class - reads an image from a device and attempts to put it out to a file.
// 1/5/12 Chris Cornelius
// skysphere project, ATP 2012
// St. Olaf College

#include "ImageCapture.h"

#include <iostream>
#include <cstring>
using std::cout;
using std::cerr;
using std::endl;

// Default video device to capture from - /dev/video1 is the right device as seen from RNS 202 or camera 2 on the remote control
//for the Sony EVI-HD1's using a blackmagic video card it appears that we'll need to use /dev/blackmagic0 and /dev/blackmagic1
static char* DEFAULT_VIDEO_DEVICE = {"/dev/blackmagic0"};
static int DEFAULT_NUMBER_PICTURES = 1;
static char* RIGHT_CAMERA = {"/dev/video1"}; //2 on the remote
static char* LEFT_CAMERA = {"/dev/video0"}; //1 on the remote
//at the moment the left and right designations here are arbitrary
static char* RIGHT_HD_CAMERA = {"/dev/blackmagic0"};
static char* LEFT_HD_CAMERA = {"/dev/blackmagic1"};

int main(int argc, char* argv[])
{
  char devname[100];
  int numPics;

  // set up device name to use -- default to DEFAULT_VIDEO_DEVICE
  if(argc > 2)
    {
      strcpy(devname, argv[2]);
    }
  else
    {
      strcpy(devname, DEFAULT_VIDEO_DEVICE);
    }

  //set up number of pictures desired -- default to DEFAULT_NUMBER_PICTURES
  if(argc >1)
    {
      numPics = atoi(argv[1]);
    }
  else
    {
      numPics = DEFAULT_NUMBER_PICTURES;
    }
  
  
  //ImageCapture ic(devname, 2);  // build object, with device_index of 2 (for S-video)
  ImageCapture right(RIGHT_HD_CAMERA, 2);
  ImageCapture left(LEFT_HD_CAMERA, 2);

  if( !left.openConnection()) // open connection
    {
      cerr << "ImageCaptureDriver: Could not open device " << LEFT_CAMERA << endl;
      return -1;
    }
  
  // assert: device is open
  cerr << "ImageCaptureDriver: opened device " << LEFT_CAMERA << " successfully." << endl;

  if( !right.openConnection()) // open connection
    {
      cerr << "ImageCaptureDriver: Could not open device " << RIGHT_CAMERA << endl;
      return -1;
    }
  
  // assert: device is open
  cerr << "ImageCaptureDriver: opened device " << RIGHT_CAMERA << " successfully." << endl;


  // capture an image
  //ic.grabImage();  // TODO: refine
  

  // capture an image the other way
  for(int i=0; i<numPics; ++i){
    left.grabImageToFile();
   cerr << "ImageCaptureDriver: grabbed left image." << endl;  
    right.grabImageToFile();
   cerr << "ImageCaptureDriver: grabbed right image." << endl; 
   sleep(1);
  }


  // clean up!
  if( !left.closeConnection()) // open connection
    {
      cerr << "ImageCaptureDriver: Could not close left connection." << endl;
      return -2;
    } 
  // assert: device closed successfully
  cerr << "ImageCaptureDriver: closed left connection successfully." << endl;

 if( !right.closeConnection()) // open connection
    {
      cerr << "ImageCaptureDriver: Could not close right connection." << endl;
      return -2;
    }
  
  // assert: device closed successfully
  cerr << "ImageCaptureDriver: closed right connection successfully." << endl;



  cerr << "ImageCaptureDriver: Testing done." << endl;
  return 0;
}
