#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_

#include <job_scheduler.h>

#include "options.h"
#include "tasks.h"

class JobScheduler {
   public:
    JobScheduler() { job_scheduler_init(); };
    ~JobScheduler() { job_scheduler_finish(); };

    JobScheduler(JobScheduler const&) = delete;
    JobScheduler& operator=(JobScheduler const&) = delete;

    void setJobConstraints(job_info_h jobInfo, const Constraints& constraints) {
        job_info_set_requires_battery_not_low(
            jobInfo, constraints.requires_battery_not_low_);
        job_info_set_requires_charging(jobInfo, constraints.requires_charging_);
    }

    void registerOneOffJob(job_info_h jobInfo, const OneoffTask& task) {
        job_info_set_once(jobInfo, true);
        
        job_scheduler_schedule(jobInfo, task.unique_name_.c_str());
    }

    void registerPeriodicJob(job_info_h jobInfo, const PeriodicTask& task) {
        job_info_set_periodic(jobInfo, task.frequency_in_seconds_);
        job_info_set_persistent(jobInfo, true);

        job_scheduler_schedule(jobInfo, task.unique_name_.c_str());
    }

    void cancelByTag(const CancelByTagTask& task) {
        // TODO : implement
    }

    void cancelByUniqueName(const CancelByUniqueNameTask& task) {
        job_scheduler_cancel(task.uniqueName_.c_str());
    }

    void cancelAll() {
        // TODO : implement
    }

    std::vector<std::string> getAllJobs() {
        std::vector<std::string> ret;

        int ret = job_scheduler_foreach_job(
            [](job_info_h job_info, void* user_data) {
                char* job_id = NULL;

                job_info_get_job_id(job_info, &job_id);

                // TODO : implement
                return true;
            },
            nullptr);
    }
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
