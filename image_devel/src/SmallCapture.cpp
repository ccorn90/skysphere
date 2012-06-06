//SmallCapture.cpp
//Charles Nye, Ian McGinnis, Chris Cornelius
//Skysphere
//Saint Olaf College
//This is a lightweight replacement for Weibe's spagetti code. It aims to be as small as possible to increase readabillity. Only essential functions are included.

////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

#define CLEAR(x) memset (&(x), 0, sizeof (x))
///////////////////////////////


using namespace std;
#include "SmallCapture.h"
#include <string>
#include <iostream>

/*
Programming a V4L2 device consists of these steps:
    -Opening the device
    Changing device properties, selecting a video input, video standard, picture brightness
    -Negotiating a data format
    -Negotiating an input/output method
    The actual input/output loop
    -Closing the device

In practice most steps are optional and can be executed out of order.
 */


// static void open_device (void)
// {
//         struct stat st; 
// 	stat (dev_name, &st);
//         S_ISCHR (st.st_mode);
//         fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
// }
// //////////////////////////////////////////////////////////////////////////////////////////
// static void
// init_device                     (void)
// {
//         struct v4l2_capability cap;
//         struct v4l2_cropcap cropcap;
//         struct v4l2_crop crop;
//         struct v4l2_format fmt;
//         unsigned int min;

// //         xioctl (fd, VIDIOC_QUERYCAP, &cap);
// //         /* Select video input, video standard and tune here. */
// 	struct v4l2_format fmt;
        
// 	CLEAR (fmt);

//         fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//         fmt.fmt.pix.width       = 640; 
//         fmt.fmt.pix.height      = 480;
//         fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
//         fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

// //         xioctl (fd, VIDIOC_S_FMT, &fmt);

//         /* Note VIDIOC_S_FMT may change width and height. */

// }
// /////////////////////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////////////////

// static void
// mainloop                        (void)
// {
//         unsigned int count;

//         count = 100;

//         while (count-- > 0) {
//                 while (true) {
//                         fd_set fds;
//                         struct timeval tv;
//                         int r;

//                         FD_ZERO (&fds);
//                         FD_SET (fd, &fds);

//                         /* Timeout. */
//                         tv.tv_sec = 2;
//                         tv.tv_usec = 0;

//                         r = select (fd + 1, &fds, NULL, NULL, &tv);

//                         }

//                         if (read_frame ())
//                                 break;
        
//                         /* EAGAIN - continue select loop. */
//                 }
//         }
// }

// ////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
class buffer {
public:
  void * start; // a pointer to the start of data
  size_t length; // unused, may help later
  char data[921600]; // an array of <1MB, just big enough for a single frame.
};


int main (int argc, char ** argv)
{
  const char* dev_name = "/dev/video1";
	//S-Video =  2
	int fd;
	fd =open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0); //consider removeing second flag

	buffer myBuffer; // Make a buffer to hold the image
	myBuffer.start=&myBuffer.data;
	myBuffer.length=921600;

	read (fd, myBuffer.start, 921600); // 921600bytes is the magic number for the bytes in a frame
	
     	struct v4l2_format fmt;

	CLEAR (fmt); // Make sure that all values are zeroed
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = 640; 
        fmt.fmt.pix.height      = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
        fmt.fmt.pix.sizeimage   = fmt.fmt.pix.height*fmt.fmt.pix.width*fmt.fmt.pix.bytesperline; // Find the area of the image and multiply it by the bytes in a line

	//write (fd, myBuffer.start, 921600) // I suspect that this is not nessisary, We already have the data in myBuffer.data. After I write out the PPM header, I should write that data to a PPM file.
	
	cout << &myBuffer << endl;
	cout << myBuffer.start << endl;
	cout << fmt.type << endl;
	cout << fmt.fmt.pix.width << endl;
	cout << fmt.fmt.pix.height << endl;
	cout << fmt.fmt.pix.bytesperline << endl;


	//ofstream
	//myFile.write(sds_bufferR, SINGLE_FRAME_SIZE);


/*
        open_device ();

        init_device ();

	read (fd, buffers[0].start, buffers[0].length)

        uninit_device ();

        close_device ();
*/
        return 0;
}


