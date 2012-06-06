/* rvcr

   GenericDatagram - a simple UDP datagram that can receive and transmit data repeatedly

*/

#ifndef GENERICDATAGRAM_H
#define GENERICDATAGRAM_H

class GenericDatagram
{
public:
  GenericDatagram(unsigned short int p); // create gd that receives on port p
  ~GenericDatagram(); // destroy

  void receive(); // wait and receive data
  void send(HostGroup *hg, unsigned short int p); // send packet to h on port p
  void send(HostGroup *hg); // send packet to h
  HostGroup & source(); // get source host of packet
  void *data(); // get pointer to data
  int size(); // get size in bytes
  void size(int s); // set size of data

private:
  void clear();

  unsigned short int port;

  char data_buffer[DATAGRAM_BUFFER_SIZE];
  ssize_t bytes_size;
  HostGroup source_host;
  
  int socket_fd;
  sockaddr_in socket_address, source_address, dest_address;
};

#endif // GENERICDATAGRAM_H
