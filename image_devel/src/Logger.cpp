/* rvcr

   Logger implementation

*/

#include <cstdlib>
#include <cstdio>

#include "Logger.h"

Logger::Logger(const char *fn)
{
  log_file = fopen(fn, "a");
  log("log started");
}

Logger::~Logger()
{
  if (log_file != NULL) fclose(log_file);
}

void Logger::log(const char * m)
{
  if (log_file != NULL) {fprintf(log_file, "%s\n", m); fflush(log_file);}
}

void Logger::log(const char * m, int i)
{
  if (log_file != NULL) {fprintf(log_file, "%s: %d\n", m, i); fflush(log_file);}
}

void Logger::last(const char * m)
{
  log(m);
  exit(EXIT_FAILURE);
}

