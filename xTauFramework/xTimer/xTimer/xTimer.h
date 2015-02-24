/*
Class to measure times

Zinonas Z.

zenon@cern.ch

Feb 2015
*/

#ifndef xTimer_xTimer_H
#define xTimer_xTimer_H

#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "AsgTools/AsgTool.h"

class xTimer{

 public:

  xTimer() {};
  ~xTimer() = default;

  void start (const std::string&);
  void stop (const std::string&);
  void report() const;

 private:

  typedef std::chrono::time_point<std::chrono::system_clock> Chronos_t;
  typedef std::chrono::duration<double> Duration_t;

  struct TimeSlot {
  TimeSlot() : 
    elapsed(0), starttime(), endtime(), running(false) {}
    Duration_t elapsed;
    Chronos_t starttime;
    Chronos_t endtime;
    bool running;
  };


  typedef std::map<std::string, TimeSlot> ChronosMap_t;  
  ChronosMap_t m_chronos_map;

};

#endif

