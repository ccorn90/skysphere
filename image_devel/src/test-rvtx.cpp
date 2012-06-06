/* RoboVision Test Transmitter

  test-rvtx image.sds hostip

  Read image.sds, enter an interactive loop:
  1. Send entire image to hostip.
    - Image ID (fake)
  2. Send specific packet to hostip.
    - Packet ID (real)
    - Image ID (fake)
  3. Stream image to host ip.
    - Starting Image ID (fake/real)
    - Number of frames
  4. Quit

*/

#include <iostream>
using namespace std;

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "config.h"
#include "rvc_types.h"

#include "Logger.h"
#include "HostGroup.h"
#include "GenericDatagram.h"

Logger *lp = 0;

static char sds_buffer[STEROIDS_BUFFER_SIZE];
static HostGroup *host_hgp = 0;
static GenericDatagram *gdp = 0;
static timespec ts_req, ts_rem;

void send_packet(unsigned short int iid, unsigned short int packid)
{
  snipes_packet *buffer_packet;
  buffer_packet = reinterpret_cast<snipes_packet *>(gdp->data());
  buffer_packet->frame_id = iid;
  buffer_packet->packet_id = packid;
  if (packid < IMAGE_PACKET_ID_FINAL)
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
  gdp->send(host_hgp);
}

void send_image(unsigned short int iid)
{
  unsigned short int curr_packet;
  for (curr_packet = 1; curr_packet <= IMAGE_PACKET_ID_FINAL; curr_packet++)
  {
    send_packet(iid, curr_packet);
    // 500
    ts_req.tv_nsec = 20000;
    nanosleep(&ts_req, &ts_rem);
  }
}

void stream_image(unsigned short int iid, unsigned int numframes)
{
  unsigned short int curr_frame = iid;
  unsigned short int i;
  for (i = 0; i < numframes; i++)
  {
    send_image(curr_frame++);
    ts_req.tv_nsec = 20000;
    nanosleep(&ts_req, &ts_rem);
  }
}

static void test_loop()
{
  char opt;
  unsigned short int iid, packid;
  unsigned int numframes;
  while (1) {
  cout << "Send Image(i), Send Packet(p), Stream Image(s), or Quit(q)? ";
  cin >> opt;
  switch (opt)
  {
  case 'i':
    cout << "Image ID (fake): ";
    cin >> iid;
    send_image(iid);
    break;
  case 'p':
    cout << "Packet ID (real): ";
    cin >> packid;
    cout << "Image ID (fake): ";
    cin >> iid;
    send_packet(iid, packid);
    break;
  case 's':
    cout << "Starting Image ID (fake/real): ";
    cin >> iid;
    cout << "Number of frames: ";
    cin >> numframes;
    stream_image(iid, numframes);
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
  }
}

static void usage()
{
  fprintf(stderr, "TEST-RVTX\ntest-rvtx image.sds hostip\n");
  exit(EXIT_FAILURE);
}

static void exit_handler()
{
  if (gdp) delete gdp;
  if (host_hgp) delete host_hgp;
  if (lp) delete lp;
}

int main(int argc, char *argv[])
{
  FILE *sds_file = NULL;
  size_t bytes_read;

  atexit(exit_handler);
  if (argc != 3) usage();
  lp = new Logger("/dev/null");
  host_hgp = new HostGroup(argv[2]);
  gdp = new GenericDatagram(RECEIVE_PORT);

  sds_file = fopen(argv[1], "rb");
  if(fread(sds_buffer, 1, STEROIDS_BUFFER_SIZE, sds_file) != STEROIDS_BUFFER_SIZE)
  {
    cerr << "RRRAAAWRRR! not enough steroids!" << endl;
    exit(EXIT_FAILURE);
  }
  fclose(sds_file);

  ts_req.tv_sec = 0;
  ts_req.tv_nsec = 0;

  test_loop();
  return 0;
}

