#ifndef FILE_M_TIME_H
#define FILE_M_TIME_H

#ifdef _WIN32
#include <windows.h>

static inline double get_time()
{
  FILETIME systime;
  ULARGE_INTEGER utime;
  GetSystemTimeAsFileTime(&systime); // resolution 100 nanoseconds
  utime.LowPart = systime.dwLowDateTime;
  utime.HighPart = systime.dwHighDateTime;

  return utime.QuadPart / 10000000.0;
}
#else
#include <sys/time.h>

static inline double get_time()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + (tv.tv_usec)/1000000.0;
}
#endif

#endif //FILE_M_TIME_H
