// config.hpp
// ATP 2012, Skysphere Project, St. Olaf College
// Licensed under Creative Commons Attribution-ShareAlike
// http://creativecommons.org/licenses/by-sa/3.0/

// Holds important configuration variables for capture from V4L2 S-Video card.
// Adapted from rvtx program, Daniel Wiebe, St. Olaf College 2010

#define DATAGRAM_BUFFER_SIZE 1472
#define STEROIDS_BUFFER_SIZE 1843201
#define IMAGE_PACKET_BASE_SIZE 1472
#define IMAGE_PACKET_FINAL_SIZE 865
#define IMAGE_HEADER_SIZE 4
#define IMAGE_PACKET_ID_FINAL 1256
#define SNORE_PACKET_SIZE 16
#define RECEIVE_PORT 6523
#define RVRR_PORT 6524
#define SEND_PORT 6523
#define RESPONSE_BASE_PORT 38600
#define ROUTER_PIDFILE "/var/run/palantir.rvcr.pid"
#define NODE_PIDFILE "/var/run/palantir.rvcn.pid"
#define SEND_DELAY_PACKET_NS 20000
#define SEND_DELAY_FRAME_NS 20000
#define SINGLE_FRAME_SIZE 921600
#define DEFAULT_SDS_TYPE 16

