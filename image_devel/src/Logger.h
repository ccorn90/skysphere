/* rvcr

   Logger - a logger

*/

#ifndef LOGGER_H
#define LOGGER_H

class Logger
{
public:
  Logger(const char *fn); // create a logger with filename
  ~Logger(); // safely destroy logger

  void log(const char * m); // log a message
  void log(const char * m, int i);
  void last(const char * m); // log a final message and exit the program
private:
  FILE * log_file;
};

#endif // LOGGER_H

