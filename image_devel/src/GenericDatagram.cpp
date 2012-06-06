/* rvcr

   GenericDatagram implementation

*/

#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "config.h"
#include "rvc_types.h"

#include "HostGroup.h"
#include "GenericDatagram.h"
#include "Logger.h"

extern Logger *lp;

GenericDatagram::GenericDatagram(unsigned short int p)
{
  port = p;

  dest_address.sin_family = AF_INET;
  dest_address.sin_port = htons(SEND_PORT);
  
  if((socket_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) lp->last("GenericDatagram socket");
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(port);
  socket_address.sin_addr.s_addr = INADDR_ANY;
  if(bind(socket_fd, (sockaddr *)&socket_address, sizeof socket_address) == -1) lp->last("GenericDatagram bind");

  clear();
}

GenericDatagram::~GenericDatagram()
{
  close(socket_fd);
}

void GenericDatagram::receive()
{
  size_t address_len;
  clear();

  address_len = sizeof source_address;
  if((bytes_size = recvfrom(socket_fd, data_buffer, DATAGRAM_BUFFER_SIZE, 0, (sockaddr *)&source_address, (socklen_t *)&address_len)) == -1) lp->last("GenericDatagram recvfrom");
  source_host.load(source_address);
}

void GenericDatagram::send(HostGroup *hg, unsigned short int p)
{
  // send TO port p
  dest_address.sin_port = htons(p);
  send(hg);
  dest_address.sin_port = htons(SEND_PORT);  // just in case side effects matter
}

void GenericDatagram::send(HostGroup *hg)
{
  int bytes_sent = 0;

  dest_address.sin_addr = hg->address();
  if((bytes_sent = sendto(socket_fd, data_buffer, bytes_size, 0, (sockaddr *)&dest_address, sizeof dest_address)) == -1) lp->last("GenericDatagram sendto");
}

HostGroup & GenericDatagram::source()
{
  return source_host;
}

void * GenericDatagram::data()
{
  return reinterpret_cast<void *>(data_buffer);
}

int GenericDatagram::size()
{
  return bytes_size;
}

void GenericDatagram::size(int s)
{
  bytes_size = s;
}

void GenericDatagram::clear()
{
  bytes_size = 0;
  memset(data_buffer, '\0', DATAGRAM_BUFFER_SIZE);
  source_host.clear();
}

