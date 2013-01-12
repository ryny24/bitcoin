#ifndef _BITCOIN_TIMER_H_
#define _BITCOIN_TIMER_H_

#include <boost/thread/thread_time.hpp>

class CTimerJob
{
public:
    virtual void operator()() = 0;
    virtual ~CTimerJob();
};

void StartTimer();
void StopTimer();
void AddTimerJob(CTimerJob *job, const boost::system_time &dur);

#endif
