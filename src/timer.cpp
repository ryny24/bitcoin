#include "timer.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

#include <map>

class CTimer {
private:
    typedef std::multimap<boost::system_time,TimerJob*> map_t;

    boost::mutex mutex;
    map_t jobs;
    boost::condition_variable condTimer;
    boost::condition_variable condStop;
    bool fExit;
    int nRunning;

    TimerJob *WaitForJob() {
        TimerJob *job = NULL;
        boost::unique_lock<boost::mutex> lock(mutex);
        while (fExit || jobs.empty() || boost::get_system_time() < jobs.begin()->first) {
            if (fExit)
                return NULL;
            if (jobs.empty())
                condTimer.wait(lock);
            else
                condTimer.timed_wait(lock, jobs.begin()->first);
        }
        job = jobs.begin()->second;
        jobs.erase(jobs.begin());
        return job;
    }

public:
    CTimer() : fExit(false), nRunning(0) {}
    ~CTimer() { Stop(); }

    void Run() {
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            nRunning++;
        }
        while(true) {
            TimerJob *job = WaitForJob();
            if (job == NULL)
                break;
            (*job)();
            delete job;
        }
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            nRunning--;
            if (nRunning == 0)
                condStop.notify_all();
        }
    }

    void Add(TimerJob *job, const boost::system_time &time) {
        boost::unique_lock<boost::mutex> lock(mutex);

        map_t::iterator it = jobs.insert(std::make_pair(time, job));
        if (it == jobs.begin())
            condTimer.notify_one();
    }

    void Stop() {
        boost::unique_lock<boost::mutex> lock(mutex);

        fExit = true;
        while (nRunning != 0)
            condStop.wait(lock);
    }
};

static CTimer timer;

void StartTimer() {
    boost::thread(&CTimer::Run, &timer);
}

void StopTimer() {
    timer.Stop();
}

void AddTimerJob(TimerJob *job, const boost::system_time &time) {
    timer.Add(job, time);
}
