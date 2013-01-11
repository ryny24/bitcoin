#ifndef _TIMER_H_
#define _TIMER_H_ 1

#include <boost/thread/thread_time.hpp>

class TimerJob {
public:
    virtual void operator()();
    virtual ~TimerJob();
};

void StartTimer();
void StopTimer();
void AddTimerJob(TimerJob *job, const boost::system_time &dur);

#endif
