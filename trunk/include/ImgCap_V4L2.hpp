// ImgCap_V4L2.hpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/08/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// A class for capturing images from an S-Video card using Video4Linux2
// Extends ImgCap_base
// Adapted from rvtx.cpp by Daniel Wiebe, St. Olaf College 2010

#ifndef _IMGCAP_V4L2_HPP_
#define _IMGCAP_V4L2_HPP_

#include "ImgCap_base.hpp"

#include "ImgCap_V4L2_config.hpp"

// hella includes from rvtx.cpp
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

#define IMAGE_HEADER_SIZE 4 //can this be deleted?
#define DEFAULT_SDS_TYPE 16 //unknown
#define SINGLE_FRAME_SIZE 921600
#define IMGCAP_V4L2_CLEAR(x) memset (&(x), 0, sizeof (x)) // sets buffers to 0 in the manner of a for loop.  TODO: make a private function!

struct ImgCap_V4L2_buffer {
        void *                  start;
        size_t                  length;
};

class ImgCap_V4L2 : public ImgCap_base
{
protected:
  int quit_signal;
  //  char *r_dev_name;
  int r_fd;
  bool cameraOn;
  struct ImgCap_V4L2_buffer *r_buffers;
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

  bool init_device(char **dev_name_p, int *fd_p, struct ImgCap_V4L2_buffer **buffers_p, unsigned int *n_buffers_p, const char *side_label);
  bool start_capturing(int *fd_p, unsigned int *n_buffers_p);
  void errno_exit(const char * s);
  int xioctl(int fd, int request, void * arg);
  void halt_capturing(int *fd_p, struct ImgCap_V4L2_buffer **buffers_p);

 public:
  ImgCap_V4L2(const char* device, int deviceIndex); // create an ImgCap_V4L2 object with a devicename
  ImgCap_V4L2(const char* device); // create an ImgCap_V4L2 object, with the first constructor, and a device index of "2" representing S-Video.
  ~ImgCap_V4L2(); // destruct the ImgCap_V4L2 object

  // Methods overridden from 
  virtual void grabImageToFile(); //saves images with fileNumber, camera, and time
  virtual int grabImageToPPM(const char * fileName); //save an image w/given filename
  virtual bool openConnection(); // prepare to receive Image structs from the device
  virtual bool closeConnection(); // elegantly break connection to the device
  virtual Image grabImage(); 
  virtual bool good();
  
};

#endif // _IMGCAP_V4L2_HPP_
