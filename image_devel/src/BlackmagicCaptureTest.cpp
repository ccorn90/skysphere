// testing the ImgCap_Blackmagic class

#include <iostream>
#include "ImgCap_Blackmagic.hpp"
using namespace std;


int main()
{
  ImgCap_Blackmagic ic ("derpdevice",6);

  
  ic.openConnection();

  ic.grabImageToFile();

  ic.closeConnection();
  
  return 0;
}
