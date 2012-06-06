// ImageCapture.cpp
// @author Chris Cornelius, Ian McGinnis, Charles Nye
// Created 01/06/2012
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Member definitions for ImageCapture class

#include "ImageCapture.h"


const bool ImageCapture::verboseErrorPrinting = false;  // static member of ImageCapture class


//some sort of error check - involving the camera hardware. Potentially unneccessary but unsure
void ImageCapture::errno_exit(const char * s)
{
        fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
        exit (EXIT_FAILURE);
}
//part of the above error check - ioctl is a hardware in/output controller
int ImageCapture::xioctl(int fd, int request, void * arg)
{
  int r;
  
  do r = ioctl (fd, request, arg);
  while (-1 == r && EINTR == errno);
  
  return r;
}

//this was void - change to bool
bool ImageCapture::init_device(char **dev_name_p, int *fd_p, struct ImageCapture_buffer **buffers_p, unsigned int *n_buffers_p, const char *side_label)
{

  /*Copied from open_device - ATP12*/

  struct stat st; 
  
  if (-1 == stat (*dev_name_p, &st)) {
    if(ImageCapture::verboseErrorPrinting) std::cerr << "Cannot identify " << side_label << "side " << *dev_name_p << ": " << errno << strerror (errno)<< std::endl;
    return false;
  }
  
  if (!S_ISCHR (st.st_mode)) {
    if(ImageCapture::verboseErrorPrinting) std::cerr << "Side " << side_label << *dev_name_p << "is no device" <<std::endl;
    return false;
  }
  
  *fd_p = open (*dev_name_p, O_RDWR /* required */ | O_NONBLOCK, 0);
  
  if (-1 == *fd_p) {
    if(ImageCapture::verboseErrorPrinting) std::cerr << "Cannot open " << side_label << "side " << *dev_name_p << ": " << errno << strerror (errno)<< std::endl;
    return false;
  }



  /* fd, dev_name, label */
  /* buffers, n_buffers */
  
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;
  /* PALANTIR */
  int index; /* index to set */
  struct v4l2_requestbuffers req;
  
  if (-1 == xioctl (*fd_p, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      if(ImageCapture::verboseErrorPrinting) std::cerr << "Side " << side_label << *dev_name_p << "is no V4L2 device" << std::endl;
      return false;
    } else {
      std::cerr << "Other 'QueryCap' error" << std::endl;
      return false;
    }
  }
  
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "Side " << side_label << *dev_name_p << "is no video capture device" << std::endl;
    return false;
  }
  
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "Side " << side_label << *dev_name_p << "does not support streaming i/o" << std::endl;
    return false;
  }
  
  /* Select video input, video standard and tune here. */

  /* Select video input to 2 (S-Video) */
  
  index = device_input_index; /* 2 is S-Video for our card */
  
  if (-1 == xioctl (*fd_p, VIDIOC_S_INPUT, &index)) {
    switch (errno) {
      
    default:
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "In method init_device(): No video inputs to select" << std::endl;
      return false;
    }
  }
  
  /* Assume standard is 0 == NTSC */
  /* Assume tuner is 0 == Television */
  
  
  CLEAR (cropcap);
  
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  if (0 == xioctl (*fd_p, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */
    
    if (-1 == xioctl (*fd_p, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
      case EINVAL:
	/* Cropping not supported. */
	break;
      default:
	/* Errors ignored. */
	break;
      }
    }
  } else {	
    /* Errors ignored. */
  }
  
  //Set video formats
  CLEAR (fmt);

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = 640; 
  fmt.fmt.pix.height      = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  
  if (-1 == xioctl (*fd_p, VIDIOC_S_FMT, &fmt)){
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "In method init_device(): Error setting data format" << std::endl;
    return false;
  }
  
  /* Note VIDIOC_S_FMT may change width and height. */

  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;
  
  /* get the width, height, format actually used */
  if (640 != fmt.fmt.pix.width || 480 != fmt.fmt.pix.height) 
    {
      if(ImageCapture::verboseErrorPrinting) std::cerr  <<"Side " << side_label << "format inconsistent size" << std::endl;
      return false;
    }
  /* assume the format is something close to rgb */
  
  /* init mmap */
  //requesting buffers. Where we declare: struct v4l2_requestbuffers req;
  ///////////////////////////////////////////////////////////////////potential issue here? can this be changed to grab the current frame? IM: 1/12/12
  CLEAR (req);

  req.count               = 4;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_MMAP;
  
  if (-1 == xioctl (*fd_p, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "Side " << side_label << *dev_name_p << " does not support memory mapping" << std::endl;
      return false; 
    } 
    else {
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "In method init_device(): Unable to initiate memory mapped buffer I/O" << std::endl;
      return false;
    }
  }
  
  if (req.count < 2) {
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "Insufficient buffer memory on side  " << side_label << *dev_name_p << std::endl;
    return false;
  }
  
  *buffers_p = (ImageCapture_buffer *)calloc (req.count, sizeof (**buffers_p));
  
  if (!*buffers_p) {
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "Side " << side_label << ": Out of memory" << std::endl;
    return false;
  }
  
  for (*n_buffers_p = 0; *n_buffers_p < req.count; ++*n_buffers_p) {
    struct v4l2_buffer buf;
    
    CLEAR (buf);
    
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index  = *n_buffers_p;
    
    if (-1 == xioctl (*fd_p, VIDIOC_QUERYBUF, &buf)){
      std::cerr << "Issue with buffer querying in VIDIOC_QUERYBUF" <<std::endl;
      return false;
    }

    
    (*buffers_p)[*n_buffers_p].length = buf.length;
    (*buffers_p)[*n_buffers_p].start =
      mmap (NULL /* start anywhere */,
	    buf.length,
	    PROT_READ | PROT_WRITE /* required */,
	    MAP_SHARED /* recommended */,
	    *fd_p, buf.m.offset);

    ////////////////////////////////could mmap be our issue? Maybe? IM: 1/12/12
    
    if (MAP_FAILED == (*buffers_p)[*n_buffers_p].start){
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "mmap" <<std::endl;
      return false;
    } 
  }
  return true;
}

bool ImageCapture::start_capturing(int *fd_p, unsigned int *n_buffers_p)
{
  /* fd, n_buffers, label */
  unsigned int i;
  enum v4l2_buf_type type;
  
  for (i = 0; i < *n_buffers_p; ++i) {
    struct v4l2_buffer buf;
    
    CLEAR (buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;
    
    if (-1 == xioctl (*fd_p, VIDIOC_QBUF, &buf)){
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "VIDIOC_QBUF" <<std::endl;
      return false;
    }
  }
		
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
 
  if (-1 == xioctl (*fd_p, VIDIOC_STREAMON, &type)){
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "VIDIOC_STREAMON" <<std::endl;
    return false;
    }
  return true;
}


void ImageCapture::halt_capturing(int *fd_p, struct ImageCapture_buffer **buffers_p)
{
  enum v4l2_buf_type type;
  unsigned int i;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl (*fd_p, VIDIOC_STREAMOFF, &type)) 
    errno_exit ("VIDIOC_STREAMOFF");

  for (i = 0; i < r_n_buffers; ++i)
    if (-1 == munmap ((*buffers_p)[i].start, (*buffers_p)[i].length))
      errno_exit ("munmap");
  free (*buffers_p);

  if (-1 == close (*fd_p))
    errno_exit ("close");
  *fd_p = -1;
}

//////////////// functions for wait_and_read_devices

bool ImageCapture::wait_for_device(int *fd_p, const char *side_label)
{
  for(;;) {
    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(*fd_p, &fds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    r = select(*fd_p + 1, &fds, NULL, NULL, &tv);

    if (-1 == r) {
      if (EINTR == errno)
        {continue;}
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "select" << std::endl;
      return false;
    }

    if (0 == r) {
      std::cerr << "Side " << side_label << " select timeout" << std::endl;
      return false;
    }

    return true;
  }
}

int ImageCapture::read_frame(int *fd_p, struct v4l2_buffer *buf_p)
{
  sleep(.1); //added so that the video card can stablize
  CLEAR (*buf_p);
  
  buf_p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf_p->memory = V4L2_MEMORY_MMAP;

  
  if (-1 == xioctl (*fd_p, VIDIOC_DQBUF, buf_p)) {
    switch (errno) {
    case EAGAIN:
      return 0;
      
      case EIO: 
  
      /* Could ignore EIO, see spec.- Palantir */
      /* fall through */
  default:
      errno_exit ("VIDIOC_DQBUF");
    }
    }
  
  //read(*fd_p,buf_p, SINGLE_FRAME_SIZE); - commented out the above if statement and used this instead - it still worked.

  /* worth this check? assert (buf.index < n_buffers);*/
  
  /*process_image (buffers[buf.index].start);*/
  
  return 1;
}



void ImageCapture::save_frames() // optional - call to capture images (i.e. for calibration)
{
  //moves memory via C methods
        memcpy(sds_bufferR+1, r_buffers[r_buf.index].start, SINGLE_FRAME_SIZE);
	
	std::ofstream rightfile;

	char fileRight[100];

	//get current time for file names
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);

	const char* rightCamera = {"/dev/video1"};
	const char* leftCamera = {"/dev/video0"};

	char buffer[80];

	//IM: %03d_%s writes the value of fileNum, using zeros to fill 3 digits
	//and then writes the contents of buffer into the file name
	if (0 == strcmp(r_dev_name, leftCamera))
	  {
	    strftime (buffer,80,"left_%X.ppm", timeinfo);
	    sprintf(fileRight,"%03d_%s",fileNum, buffer); 
	  }  
	else if (0 == strcmp(r_dev_name, rightCamera)){
	  strftime (buffer,80,"right_%X.ppm", timeinfo);
	  sprintf(fileRight,"%03d_%s",fileNum, buffer);
	  }
	  else{
	    sprintf(fileRight,"image_%03d.ppm",fileNum);
	  }	

	rightfile.open(fileRight);
 	rightfile << "P6\n640 480\n255\n";
	rightfile.write(sds_bufferR, SINGLE_FRAME_SIZE);
	rightfile.close();

	fileNum++;
}

void ImageCapture::save_frames(const char * fileName) //for saving with a given file name
{
  //moves memory via C methods
        memcpy(sds_bufferR+1, r_buffers[r_buf.index].start, SINGLE_FRAME_SIZE);
	
	std::ofstream rightfile;

	char fileRight[100];

	//IM: %03d_%s writes the value of fileNum, using zeros to fill 3 digits
	//and then writes the contents of buffer into the file name

	sprintf(fileRight,fileName);	

	rightfile.open(fileRight);
 	rightfile << "P6\n640 480\n255\n";
	rightfile.write(sds_bufferR, SINGLE_FRAME_SIZE);
	rightfile.close();

}

///////////////


// create an ImageCapture object with a devicename
// device names are:/dev/video0 or /dev/video1 and the typical deviceIndex is:2 
ImageCapture::ImageCapture(char* device, int deviceIndex){

  r_dev_name = new char[strlen(device)];
  strcpy(r_dev_name,device);
  device_input_index = deviceIndex;
  quit_signal = 0;
  r_fd = -1;
  r_buffers = NULL;
  r_n_buffers = 0;
  frame_id = 1;
  fileNum = 0;
}
// create an ImageCapture object, with the first constructor, and a device index of "2" representing S-Video.
ImageCapture::ImageCapture(char* device){ 
  r_dev_name = new char[strlen(device)];
  strcpy(r_dev_name,device);
  quit_signal = 0;
  device_input_index = 2;  // default value of two;
  r_fd = -1;
  r_buffers = NULL;
  r_n_buffers = 0;
  frame_id = 1;
  fileNum =0;
}
// destruct the ImageCapture object
ImageCapture::~ImageCapture(){}


//set all up
bool ImageCapture::openConnection(){

  if(!init_device(&r_dev_name, &r_fd, &r_buffers, &r_n_buffers, "right"))
    {return false;}
  cameraOn = true;
  start_capturing(&r_fd, &r_n_buffers);
  return true;
}

//tear all down
bool ImageCapture::closeConnection(){
  if (true == cameraOn){
  halt_capturing(&r_fd, &r_buffers);
  cameraOn = false;
  return true;
  }
  else return true;

}

//return a captured image - from wait_and_read_devices
void ImageCapture::grabImageToFile(){
  /* 
     1. wait on both devices
     2. read the device that was ready first
     3. wait on the second device
     4. read the second device

  */
  fd_set fds;
  while(true) {
      struct timeval tv;
      int r;

    FD_ZERO(&fds);
//cn/im: adds a file descriptor to a file descriptor set
    FD_SET(r_fd, &fds);
 
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    r = select(r_fd+1, &fds, NULL, NULL, &tv);
    //cn/im: select() is a socket function which monitors a list of file descriptors for readability,so here we have (point to array of file descriptors, number of file descriptors checked, don't check for writeable, no files are checked for exceptions thrown, and a timeout value)

    //these check to see if a camera is connected, if one isn't after one second it throws a connection timeout
  if (-1 == r) {
      if (EINTR == errno)
        continue;
      std::cerr << "select" << std::endl;
      //errno_exit ("select");
    }

    if (0 == r) {
      std::cerr<< "Master select timeout" << std::endl;
      exit (EXIT_FAILURE);
    }

    break;

  }  
    wait_for_device(&r_fd, "right");
    read_frame(&r_fd, &r_buf);
    save_frames();
}




//these are going to replace save_frames in the above method

/*
Image ImageCapture::grabImage() //maybe grabFrame?
{
  Image temp(640,480,Image::CHAR_CHAN, 3); //width, height, channel type, num channels

  memcpy(temp.getData(), r_buffers[r_buf.index].start, SINGLE_FRAME_SIZE);

  return temp;
}
*/
/*
Image ImageCapture::writeImage() //writeFrame?
{

        std::ostream rightfile;
        char fileRight[15];

        sprintf(fileRight, "%03d_right.ppm",fileNum);

	rightfile.open(fileRight);
 	rightfile << "P6\n640 480\n255\n";
	rightfile.write(grabImage(), SINGLE_FRAME_SIZE); //this is an issue
	rightfile.close();
	std::cout << "image save" << std::endl; //added for trouble shooting

	fileNum++;

}

*/

int ImageCapture::grabImageToPPM(const char * fileName){
  /* 
     1. wait on device
     2. read the device 
  */
  fd_set fds;
  while(true) {
    struct timeval tv;
    int r;
    
    FD_ZERO(&fds);
    //cn/im: adds a file descriptor to a file descriptor set
    FD_SET(r_fd, &fds);
    
    tv.tv_sec = 1;
    tv.tv_usec = 0;
   
    //check for readability 
    r = select(r_fd+1, &fds, NULL, NULL, &tv);
    
    //these check to see if a camera is connected, if one isn't after one second it throws a connection timeout
    if (-1 == r) {
      if (EINTR == errno)
        continue;
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "select" <<std::endl;
      return 1;
    }
    
    if (0 == r) {
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "Master select timeout" << std::endl;
      return 2;
    }
    
    break;
    
  }  
  wait_for_device(&r_fd, "right");
  read_frame(&r_fd, &r_buf);
  save_frames(fileName);
  
  return 0;
}

bool ImageCapture::good() // returns true if works, false if connections unestablished
{
  fd_set fds;

  struct timeval tv;
  int r;

  //deletes all file descriptors from fds
  FD_ZERO(&fds);
  //adds a file descriptor
  FD_SET(r_fd, &fds);

  //set delay time  
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  
  r = select(r_fd+1, &fds, NULL, NULL, &tv);
  
  //these check to see if a camera is connected, if one isn't after one second it throws a connection timeout
  if (-1 == r) {
    if (EINTR == errno)
      if(ImageCapture::verboseErrorPrinting) std::cerr  << "In method good(): Unreadable device" << std::endl;
    return false;
  }
  
  if (0 == r) {
    if(ImageCapture::verboseErrorPrinting) std::cerr  << "In method good(): Device timed out" << std::endl;
    return false;
  }

  return true;
}

