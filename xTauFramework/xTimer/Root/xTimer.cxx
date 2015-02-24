#include <xTimer/xTimer.h>

void xTimer::start (const std::string& procname) {

  auto it = m_chronos_map.find(procname); //ChronosMapItr_t

  if(it == m_chronos_map.end()){
    
    TimeSlot s;
    s.starttime = std::chrono::system_clock::now();
    s.running = true;
    
    m_chronos_map.emplace( procname,  s);

  }else{
    Warning("Timer::start()", "Process %s is already registered", procname.c_str());
  }

}

void xTimer::stop (const std::string& procname) {

  auto it = m_chronos_map.find(procname);

  if(it != m_chronos_map.end()){
    
    TimeSlot &slot = m_chronos_map[procname];
    if(slot.running){
      slot.endtime = std::chrono::system_clock::now();
      slot.elapsed = (slot.endtime - slot.starttime);
    }else{
      Warning("Timer::stop()", "Trying to stop time for non runnning process %s", procname.c_str());
    }


  }else{
    Warning("Timer::stop()", "Process %s is not registered", procname.c_str());
  }

}

void xTimer::report() const {

  Info("Timer::report()", "Elapsed times");
  for (const auto &t : m_chronos_map)
    std::cout << std::setw(15) <<t.first << ":" << std::setw(15) << t.second.elapsed.count() << " s\n";
}

