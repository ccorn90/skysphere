/* rvcr

   HostGroup - either a single host or a subnet

   does not store port

*/

#ifndef HOSTGROUP_H
#define HOSTGROUP_H

class HostGroup
{
public:
  HostGroup(); // create a cleared hg
  HostGroup(const char *a); // create hg with address only
  HostGroup(const char *a, const char *s); // create hg with address and subnet

  void load(sockaddr_in &si);

  bool is_clear(); // see if hg is clear
  void clear(); // clear the hg

  in_addr &address();

  bool operator==(HostGroup &rhs); // test for hg equality on single host
  bool operator<(HostGroup &rhs); // test for hg membership on subnet group

private:
  void load(const char *a, const char *s);

  in_addr host_address;
  in_addr host_subnet;
};

#endif // HOSTGROUP_H

