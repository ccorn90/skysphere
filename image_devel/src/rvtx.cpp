/* RVTX
 * RoboVision Transmit
 *
 * rvtx leftdev rightdev input cluster_ip
 *
 * Begin capturing frames from the two devices and send them to the cluster
 *
 * Todd Frederick - 04/29/2009
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <fstream>
using namespace std;

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
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include "config.h"
#include "rvc_types.h"

#include "Logger.h"
#include "HostGroup.h"
#include "GenericDatagram.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
        void *                  start;
        size_t                  length;
};

struct timeval start, end; //added for latency testing

Logger *lp = 0;

static const bool is_debug = 0;
static int quit_signal = 0;
static char *l_dev_name = NULL;
static char *r_dev_name = NULL;
static int device_input_index = 0;
static HostGroup *cluster_hgp = 0;
static GenericDatagram *gdp = 0;
static int l_fd = -1;
static int r_fd = -1;
static struct buffer *         l_buffers         = NULL;
static unsigned int     l_n_buffers       = 0;
static struct buffer *         r_buffers         = NULL;
static unsigned int     r_n_buffers       = 0;
static struct v4l2_buffer l_buf;
static struct v4l2_buffer r_buf;
static timespec ts_delay, ts_rem;
static unsigned short int frame_id = 1;
static char sds_buffer[STEROIDS_BUFFER_SIZE];
long mtime, seconds, useconds; //added for latency testing
static char sds_bufferL[STEROIDS_BUFFER_SIZE]; // added for save
static char sds_bufferR[STEROIDS_BUFFER_SIZE]; // added for save

static void stopclock()
{
		gettimeofday(&end, NULL); // added for latency testing
		seconds  = end.tv_sec - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
		printf("Time to capture and send pair: %ld milliseconds\n", mtime);
}

double getCurrentTime()
{
	struct timeval tv = {0,0};
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec + tv.tv_usec/(double)1000000.;
}


unsigned int fileNum =0;

static void save_frames() // optional - call to capture images (i.e. for calibration)
{

	memcpy(sds_bufferL+1, l_buffers[l_buf.index].start, SINGLE_FRAME_SIZE);
        memcpy(sds_bufferR+1, r_buffers[r_buf.index].start, SINGLE_FRAME_SIZE);
	
	ofstream leftfile, rightfile;

	char fileLeft[100],fileRight[100];

	//IM: the below writes fileNum in for %03d - I believe 03 tells it to write 3 digits
	sprintf(fileLeft,"%03d_left.ppm",fileNum);
	sprintf(fileRight,"%03d_right.ppm",fileNum);
	

	leftfile.open(fileLeft);
	rightfile.open(fileRight);
        leftfile << "P6\n640 480\n255\n";		//cn:Header info for the left ppm 
	//(size will be 640x480 with no HDR255)
	rightfile << "P6\n640 480\n255\n";		//cn:Header info for the right ppm
	leftfile.write(sds_bufferL, SINGLE_FRAME_SIZE);
	rightfile.write(sds_bufferR, SINGLE_FRAME_SIZE);
	leftfile.close();
	rightfile.close();

	fileNum++;			//CN: Increments for next filename.
}

static void send_packet(unsigned short int packid)
{
  snipes_packet *buffer_packet;
  buffer_packet = reinterpret_cast<snipes_packet *>(gdp->data());
  buffer_packet->frame_id = frame_id;
  buffer_packet->packet_id = packid;
/*  buffer_packet->value1 = 0;
  buffer_packet->value2 = 0;
  buffer_packet->value3 = 0;
*/  if (packid < IMAGE_PACKET_ID_FINAL)
  {
    memcpy(buffer_packet->data, sds_buffer+((packid-1)*(IMAGE_PACKET_BASE_SIZE-IMAGE_HEADER_SIZE)), (IMAGE_PACKET_BASE_SIZE-IMAGE_HEADER_SIZE));
    gdp->size(IMAGE_PACKET_BASE_SIZE);
  }
  else if (packid == IMAGE_PACKET_ID_FINAL)
  {
    memcpy(buffer_packet->data, sds_buffer+((packid-1)*(IMAGE_PACKET_BASE_SIZE-IMAGE_HEADER_SIZE)), (IMAGE_PACKET_FINAL_SIZE-IMAGE_HEADER_SIZE));
    gdp->size(IMAGE_PACKET_FINAL_SIZE);
  }
  else return;
   
  gdp->send(cluster_hgp);
}

static void send_to_cluster(void)
{
  unsigned short int curr_packet;

  unsigned char *buffer_pointer_for_type = reinterpret_cast<unsigned char *>(sds_buffer);
  *buffer_pointer_for_type = DEFAULT_SDS_TYPE;

  memcpy(sds_buffer+1, l_buffers[l_buf.index].start, SINGLE_FRAME_SIZE);
  memcpy(sds_buffer+1+SINGLE_FRAME_SIZE, l_buffers[l_buf.index].start, SINGLE_FRAME_SIZE);

  for (curr_packet = 1; curr_packet <= IMAGE_PACKET_ID_FINAL; curr_packet++)
  {
    send_packet(curr_packet);
    ts_delay.tv_nsec = SEND_DELAY_PACKET_NS;
    nanosleep(&ts_delay, &ts_rem);
  }
  ts_delay.tv_nsec = SEND_DELAY_FRAME_NS;
  nanosleep(&ts_delay, &ts_rem);
}

static void
errno_exit                      (const char *           s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

static int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

static void halt_capturing(int *fd_p, struct buffer **buffers_p, unsigned int *n_buffers_p)
{
  enum v4l2_buf_type type;
  unsigned int i;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl (*fd_p, VIDIOC_STREAMOFF, &type)) 
    errno_exit ("VIDIOC_STREAMOFF");

  for (i = 0; i < *n_buffers_p; ++i)
    if (-1 == munmap ((*buffers_p)[i].start, (*buffers_p)[i].length))
      errno_exit ("munmap");
  free (*buffers_p);

  if (-1 == close (*fd_p))
    errno_exit ("close");
  *fd_p = -1;
}

static void finish_read_frame(int *fd_p, struct v4l2_buffer *buf_p)
{
	if (-1 == xioctl (*fd_p, VIDIOC_QBUF, buf_p))
		errno_exit ("VIDIOC_QBUF");
}


static int read_frame(int *fd_p, struct v4l2_buffer *buf_p)
{
	CLEAR (*buf_p);

        buf_p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf_p->memory = V4L2_MEMORY_MMAP;

    	if (-1 == xioctl (*fd_p, VIDIOC_DQBUF, buf_p)) {
        	switch (errno) {
            	case EAGAIN:
                	return 0;

		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */
		default:
			errno_exit ("VIDIOC_DQBUF");
		}
	}

	/* worth this check? assert (buf.index < n_buffers);*/

	/*process_image (buffers[buf.index].start);*/

	return 1;
}

static void wait_for_device(int *fd_p, const char *side_label)
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
        continue;
      errno_exit ("select");
    }

    if (0 == r) {
      fprintf (stderr, "Side %s: select timeout\n", side_label);
      exit (EXIT_FAILURE);
    }

    break;
  }
}

static void wait_and_read_devices(void)
{
  /* 
     1. wait on both devices
     2. read the device that was ready first
     3. wait on the second device
     4. read the second device

  */
  fd_set fds;

  for(;;) {
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(l_fd, &fds); //cn/im: adds a file descriptor to a file descriptor set
    FD_SET(r_fd, &fds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    r = select((l_fd > r_fd ? l_fd : r_fd) + 1, &fds, NULL, NULL, &tv);
    //cn/im: select() is a socket function which monitors a list of file descriptors for readability,
    // readiness for writing, and exception pending conditions

    if (-1 == r) {
      if (EINTR == errno)
        continue;
      errno_exit ("select");
    }

    if (0 == r) {
      fprintf (stderr, "Master select timeout\n");
      exit (EXIT_FAILURE);
    }

    break;
  }

  if (FD_ISSET(l_fd, &fds))
  {
    /* case 1: left is ready, wait and read right */
    read_frame(&l_fd, &l_buf);
    wait_for_device(&r_fd, "right");
    read_frame(&r_fd, &r_buf);
		save_frames();
  }
  else
  {
    assert(FD_ISSET(r_fd, &fds));
    /* case 2: right is ready, wait and read left */
    read_frame(&r_fd, &r_buf);
    wait_for_device(&l_fd, "left");
    read_frame(&l_fd, &l_buf);
		save_frames();
  }
}

static void mainloop(void)
{
  int inputi;
        while (!quit_signal) {
		// DEBUG
		if (is_debug) 
                {
                  fscanf(stdin, "%d", &inputi);
		  //                  ts_delay.tv_nsec = 30000;
		  if (inputi == 0){
		    break;
		  }
                  ts_delay.tv_sec = 1;
                  nanosleep(&ts_delay, &ts_rem);
                }
		
		gettimeofday(&start, NULL); // added for latency testing
                
                wait_and_read_devices();
								// send_to_cluster(); // skipped for Open House demo
		finish_read_frame(&l_fd, &l_buf);
		finish_read_frame(&r_fd, &l_buf);

                if (frame_id == USHRT_MAX)
                {
                  frame_id = 1;
                }
                else
                {
                  frame_id++;
                }
                if (is_debug) fprintf(stderr, "SEND\n"); // DEBUG
		stopclock(); //added for latency testing
		//	quit_signal=true;  //ADDED BY CN/IM to force to aquire only one Image.
	}

	fprintf(stderr, "\nQuitting...\n");
	halt_capturing(&l_fd, &l_buffers, &l_n_buffers);
	halt_capturing(&r_fd, &r_buffers, &r_n_buffers);
  	exit (EXIT_SUCCESS);
}

static void start_capturing(int *fd_p, unsigned int *n_buffers_p)
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

        	if (-1 == xioctl (*fd_p, VIDIOC_QBUF, &buf))
                    	errno_exit ("VIDIOC_QBUF");
	}
		
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (*fd_p, VIDIOC_STREAMON, &type))
		errno_exit ("VIDIOC_STREAMON");
}

static void init_device(char **dev_name_p, int *fd_p, struct buffer **buffers_p, unsigned int *n_buffers_p, const char *side_label)
{
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
                        fprintf (stderr, "Side %s %s is no V4L2 device\n",
                                 side_label, *dev_name_p);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf (stderr, "Side %s %s is no video capture device\n",
                         side_label, *dev_name_p);
                exit (EXIT_FAILURE);
        }

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "Side %s %s does not support streaming i/o\n",
			 side_label, *dev_name_p);
		exit (EXIT_FAILURE);
	}

        /* Select video input, video standard and tune here. */

	/* Select video input to 2 (S-Video) */
	
	index = device_input_index; /* 2 is S-Video for our card */

	if (-1 == xioctl (*fd_p, VIDIOC_S_INPUT, &index)) {
		switch (errno) {

		default:
			errno_exit ("VIDIOC_S_INPUT");
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


        CLEAR (fmt);

        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = 640; 
        fmt.fmt.pix.height      = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl (*fd_p, VIDIOC_S_FMT, &fmt))
                errno_exit ("VIDIOC_S_FMT");

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
		fprintf (stderr, "Side %s format inconsistent size\n", side_label);
		exit(EXIT_FAILURE);
        }
	/* assume the format is something close to rgb */

	/* init mmap */

        CLEAR (req);

        req.count               = 4;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (*fd_p, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "Side %s %s does not support "
                                 "memory mapping\n", side_label, *dev_name_p);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf (stderr, "Insufficient buffer memory on side %s %s\n",
                         side_label, *dev_name_p);
                exit (EXIT_FAILURE);
        }

        *buffers_p = (buffer *)calloc (req.count, sizeof (**buffers_p));

        if (!*buffers_p) {
                fprintf (stderr, "Side %s: Out of memory\n", side_label);
                exit (EXIT_FAILURE);
        }

        for (*n_buffers_p = 0; *n_buffers_p < req.count; ++*n_buffers_p) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = *n_buffers_p;

                if (-1 == xioctl (*fd_p, VIDIOC_QUERYBUF, &buf))
                        errno_exit ("VIDIOC_QUERYBUF");

                (*buffers_p)[*n_buffers_p].length = buf.length;
                (*buffers_p)[*n_buffers_p].start =
                        mmap (NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              *fd_p, buf.m.offset);

                if (MAP_FAILED == (*buffers_p)[*n_buffers_p].start)
                        errno_exit ("mmap");
        }
}

static void open_device(char **dev_name_p, int *fd_p, const char *side_label)
{
	/* dev_name, fd */

        struct stat st; 

        if (-1 == stat (*dev_name_p, &st)) {
                fprintf (stderr, "Cannot identify %s side '%s': %d, %s\n",
                         side_label, *dev_name_p, errno, strerror (errno));
                exit (EXIT_FAILURE);
        }

        if (!S_ISCHR (st.st_mode)) {
                fprintf (stderr, "Side %s %s is no device\n", side_label, *dev_name_p);
                exit (EXIT_FAILURE);
        }

        *fd_p = open (*dev_name_p, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == *fd_p) {
                fprintf (stderr, "Cannot open %s side '%s': %d, %s\n",
                         side_label, *dev_name_p, errno, strerror (errno));
                exit (EXIT_FAILURE);
        }
}

static void usage(void)
{
  fprintf (stderr,"RVTX\nrvtx leftdev rightdev input host_ip\n");
  exit(EXIT_FAILURE);
}

static void quit_handler(int s)
{
  quit_signal = 1;
}

static void exit_handler()
{
  if (cluster_hgp) delete cluster_hgp;
  if (gdp) delete gdp;
  if (lp) delete lp;
}

int main(int argc, char *argv[])
{
  struct sigaction sa;
  atexit(exit_handler);
  signal(SIGQUIT, quit_handler);
  sigaction(SIGQUIT, NULL, &sa);
  sa.sa_flags &= ~SA_RESTART;
  sigaction(SIGQUIT, &sa, NULL);

  ts_delay.tv_sec = 0;
  ts_delay.tv_nsec = 0;

  if (5 != argc) usage();
  l_dev_name = argv[1];
  r_dev_name = argv[2];
  device_input_index = atoi(argv[3]);
  cluster_hgp = new HostGroup(argv[4]);
  gdp = new GenericDatagram(RECEIVE_PORT);
  open_device(&l_dev_name, &l_fd, "left");
  open_device(&r_dev_name, &r_fd, "right");
  init_device(&l_dev_name, &l_fd, &l_buffers, &l_n_buffers, "left");
  init_device(&r_dev_name, &r_fd, &r_buffers, &r_n_buffers, "right");
  start_capturing(&l_fd, &l_n_buffers);
  start_capturing(&r_fd, &r_n_buffers);
  mainloop();
  return 0;
}

