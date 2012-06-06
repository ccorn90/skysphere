// ImageCapture.hpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/08/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// A class for capturing images from an S-Video card using Video4Linux2
// Adapted from rvtx.cpp by Daniel Wiebe, St. Olaf College 2010

#ifndef _IMAGECAPTURE_H_
#define _IMAGECAPTURE_H_

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <fstream>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

//library used for interfacing with Prof. Olaf's eriol program
#include "eriolHeader.h"

#include "ImageCapture_config.h"

#define IMAGE_HEADER_SIZE 4 //can this be deleted?
#define DEFAULT_SDS_TYPE 16 //unknown
#define SINGLE_FRAME_SIZE 921600
#define CLEAR(x) memset (&(x), 0, sizeof (x)) //this sets buffers to 0 in the manner of a for loop. To be replaced later

struct ImageCapture_buffer {
        void *                  start;
        size_t                  length;
};

class ImageCapture
{
  
  // a variable for controlling error printing in the cpp file associated with these objects
  const static bool verboseErrorPrinting;
  
 private:
  int quit_signal;
  char *r_dev_name;
  int device_input_index;
  int r_fd;
  bool cameraOn;
  struct ImageCapture_buffer *r_buffers;
  unsigned int r_n_buffers;
  struct v4l2_buffer r_buf;
  timespec ts_delay, ts_rem;
  unsigned short int frame_id;
  char sds_buffer[STEROIDS_BUFFER_SIZE];
  char sds_bufferR[STEROIDS_BUFFER_SIZE]; // added for save
  unsigned int fileNum;
  //the below methods were taken from wiebe's work on Palantir10
  void save_frames();
  void save_frames(const char * fileName);
  int read_frame(int *fd_p, struct v4l2_buffer *buf_p);
  bool wait_for_device(int *fd_p, const char *side_label);

  bool init_device(char **dev_name_p, int *fd_p, struct ImageCapture_buffer **buffers_p, unsigned int *n_buffers_p, const char *side_label);
  bool start_capturing(int *fd_p, unsigned int *n_buffers_p);
  void errno_exit(const char * s);
  int xioctl(int fd, int request, void * arg);
  void halt_capturing(int *fd_p, struct ImageCapture_buffer **buffers_p);

 public:
  ImageCapture(char* device, int deviceIndex); // create an ImageCapture object with a devicename
  ImageCapture(char* device); // create an ImageCapture object, with the first constructor, and a device index of "2" representing S-Video.
  ~ImageCapture(); // destruct the ImageCapture object
  void grabImageToFile(); //saves images with fileNumber, camera, and time
  int grabImageToPPM(const char * fileName); //save an image w/given filename
  bool openConnection(); // prepare to receive Image structs from the device
  bool closeConnection(); // elegantly break connection to the device
  Image grabImage(); 
  //Image writeImage();
  bool good();

  char* device() { return r_dev_name; }
  
};

#endif // _IMAGECAPTURE_H_
