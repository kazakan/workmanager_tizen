#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_ 

#include <job_scheduler.h>

#include "options.h"
#include "tasks.h"

class JobScheduler {
   public:
    JobScheduler(){};
    ~JobScheduler(){};

    JobScheduler(JobScheduler const&) = delete;
    JobScheduler& operator=(JobScheduler const&) = delete;

    void setJobConstraints(job_info_h jobInfo, const Constraints& constraints) {
        job_info_set_requires_battery_not_low(
            jobInfo, constraints.requiresBatteryNotLow);

        job_info_set_requires_charging(jobInfo, constraints.requiresCharging);
    }

    void registerOneOffJob(job_info_h jobInfo, const OneoffTask& task) {
        job_info_set_once(jobInfo, true);

        job_scheduler_schedule(jobInfo, task.uniquename.c_str());
    }

    void registerPeriodicJob(job_info_h jobInfo, const PeriodicTask& task) {
        job_info_set_periodic(jobInfo, task.frequencyInSeconds);
        job_info_set_persistent(jobInfo, true);

        job_scheduler_schedule(jobInfo, task.uniquename.c_str());
    }

    void cancelByTag(const CancelByTagTask& task) {}

    void cancelByUniqueName(const CancelByUniqueNameTask& task) {
        job_scheduler_cancel(task.uniqueName.c_str());
    }
};

#endif // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
