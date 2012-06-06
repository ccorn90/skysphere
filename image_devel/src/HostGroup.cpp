/* rvcr

   HostGroup implementation

*/

#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "HostGroup.h"
#include "Logger.h"

extern Logger *lp;

HostGroup::HostGroup()
{
  host_address.s_addr = 0;
  host_subnet.s_addr = 0;
}

HostGroup::HostGroup(const char *a)
{
  load(a, "0.0.0.0");
}

HostGroup::HostGroup(const char *a, const char *s)
{
  load(a, s);
}

void HostGroup::load(sockaddr_in &si)
{
  clear();
  host_address.s_addr = si.sin_addr.s_addr;
}

bool HostGroup::is_clear()
{
  return host_address.s_addr == 0 && host_subnet.s_addr == 0;
}

void HostGroup::clear()
{
  host_address.s_addr = 0;
  host_subnet.s_addr = 0;
}

in_addr &HostGroup::address()
{
  return host_address;
}

bool HostGroup::operator==(HostGroup &rhs)
{
  return host_address.s_addr == rhs.host_address.s_addr;
}

bool HostGroup::operator<(HostGroup &rhs)
{
  return (host_address.s_addr & rhs.host_subnet.s_addr) == (rhs.host_address.s_addr & rhs.host_subnet.s_addr);
}

void HostGroup::load(const char *a, const char *s)
{
  if(inet_pton(AF_INET, a, &host_address) <= 0) lp->last("HostGroup pton");
  if(inet_pton(AF_INET, s, &host_subnet) <= 0) lp->last("HostGroup pton");
}

