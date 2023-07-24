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

    void SetJobConstraints(job_info_h job_info, const Constraints& constraints) {
        job_info_set_requires_battery_not_low(
            job_info, constraints.requires_battery_not_low_);
        job_info_set_requires_charging(job_info, constraints.requires_charging_);
    }

    void RegisterOneOffJob(job_info_h job_info, const OneoffTask& task) {
        job_info_set_once(job_info, true);

        job_scheduler_schedule(job_info, task.unique_name_.c_str());
    }

    void RegisterPeriodicJob(job_info_h job_info, const PeriodicTask& task) {
        job_info_set_periodic(job_info, task.frequency_in_seconds_);
        job_info_set_persistent(job_info, true);

        job_scheduler_schedule(job_info, task.unique_name_.c_str());
    }

    void CancelByTag(const CancelByTagTask& task) {
        // TODO : implement
    }

    void CancelByUniqueName(const CancelByUniqueNameTask& task) {
        job_scheduler_cancel(task.uniqueName_.c_str());
    }

    void CancelAll() {
        // TODO : implement
    }

    std::vector<std::string> GetAllJobs() {
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
