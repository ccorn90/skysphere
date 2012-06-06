// serialtest.c
// Attempts to open and close a serial port.
// From code found online 1/4/2012
// Adapted by C. Cornelius, ATP 2012, skysphere project
// St. Olaf College

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h> 

int open_port(char*); 
 
int main(int argc, char* argv[])
{
 int fd=-1;     // File descriptor
 char filename[100];  // buffer for filename
 struct termios options;

 if(argc > 1)
  {
     strcpy(filename,argv[1]);
  }
 else
  {
     strcpy(filename,"/dev/ttyUSB0");
  }
 
 fd = open_port(filename);

 if(fd == -1) return -1;

 fprintf(stderr,"Configuring serial port with baud rate 9600, 8bits, no parity.\n");
 
 fcntl(fd, F_SETFL, FNDELAY);                  /* Configure port reading */
                                     /* Get the current options for the port */
 tcgetattr(fd, &options);
 cfsetispeed(&options, B19200);                 /* Set the baud rates to 9600 */
 cfsetospeed(&options, B19200);
    
                                   /* Enable the receiver and set local mode */
 options.c_cflag |= (CLOCAL | CREAD);
 options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
 options.c_cflag &= ~CSTOPB;
 options.c_cflag &= ~CSIZE;
 options.c_cflag |=  CS8;                              /* Select 8 data bits */
 options.c_cflag &= ~CRTSCTS;               /* Disable hardware flow control */  

                                 /* Enable data to be processed as raw input */
 options.c_lflag &= ~(ICANON | ECHO | ISIG);
       
                                        /* Set the new options for the port */
 tcsetattr(fd, TCSANOW, &options);

 /* EXAMPLE ON HOW TO WRITE TO PORT -- COMMENTED OUT
 char* buff = new char;
 (*buff) = 0xE6;
                         
 while(1)
 {
   n = write(fd, buff,1);
   n = write(fd, buff,1);
   n = write(fd, buff,1);

 }
  */

  /* Close the serial port */
  int i;
  i = close(fd);
  fprintf(stderr,"Closed serial port with status %i\n", i);
 }

 
 int open_port(char* portname)
 {
   int fd;                                   /* File descriptor for the port */

   fprintf(stderr, "Opening device %s\n", portname);
   fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);

   if (fd == -1)
   {                                              /* Could not open the port */
     fprintf(stderr, "Unable to open %s\n",portname);
     fprintf(stderr, "Perhaps you do not have access to this device?  Check permissions\non %s ... you need rw access. Maybe add yourself to the group?\nThe default group is uucp for USB-serial devices.\n",portname);
   }
   else
   {
     fprintf(stderr, "Successfully opened %s\n", portname);
   }

   return (fd);
 }
