#ifndef _GET_PID_H_
#define _GET_PID_H_
#pragma once

#ifdef _WIN32
#include <windows.h>

int get_pid()
{
  return (int)GetCurrentProcessId();
}

#else
#include <unistd.h>

int get_pid()
{
  return (int)getpid();
}

#endif //_WIN32
#endif //_GET_PID_H_
