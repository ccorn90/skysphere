/* RoboVision Cluster types */

struct snipes_packet
{
  unsigned short int frame_id;
  unsigned short int packet_id;
//  unsigned int value1;
//  unsigned int value2;
 // unsigned int value3;
  unsigned char data[1];
};

struct snore_packet
{
  unsigned short int frame_id;
  unsigned int value1;
  unsigned int value2;
};

struct result_coord
{
  unsigned short int col;
  unsigned short int row;
};

void manage_daemon(int mode);

