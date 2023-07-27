#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_

#include <job_scheduler.h>

#include <memory>

#include "options.h"
#include "tasks.h"

class JobScheduler {
   public:
    JobScheduler(JobScheduler const&) = delete;
    JobScheduler& operator=(JobScheduler const&) = delete;

    static std::shared_ptr<JobScheduler> instance() {
        if (!instance_) {
            instance_ = std::make_shared<JobScheduler>();
        }
        return instance_;
    }

    void SetJobConstraints(job_info_h job_info,
                           const Constraints& constraints) {
        job_info_set_requires_battery_not_low(job_info,
                                              constraints.battery_not_low);
        job_info_set_requires_charging(job_info, constraints.charging);
    }

    void RegisterJob(const RegisterTaskInfo& task,
                     const bool isPeriodic = false) {
        job_info_h job_info;
        job_info_create(&job_info);

        if (isPeriodic) {
            job_info_set_periodic(job_info,
                                  task.frequency_in_seconds.value_or(1));
            job_info_set_persistent(job_info, true);
        } else {
            job_info_set_once(job_info, true);
        }

        job_scheduler_schedule(job_info, task.unique_name.c_str());
        job_info_destroy(job_info);
    }

    void CancelByTag(const std::string& task) {
        // TODO : implement
    }

    void CancelByUniqueName(const std::string& name) {
        job_scheduler_cancel(name.c_str());
    }

    void CancelAll() { job_scheduler_cancel_all(); }

    std::vector<std::string> GetAllJobs() {
        std::vector<std::string> jobs;

        int code = job_scheduler_foreach_job(
            [](job_info_h job_info, void* user_data) {
                char* job_id = NULL;
                auto vec = static_cast<std::vector<std::string>*>(user_data);

                job_info_get_job_id(job_info, &job_id);
                vec->emplace_back(job_id);
                return true;
            },
            &jobs);

        return jobs;
    }

   private:
    JobScheduler() { job_scheduler_init(); };
    ~JobScheduler() { job_scheduler_finish(); };
    static std::shared_ptr<JobScheduler> instance_;
};

std::shared_ptr<JobScheduler> JobScheduler::instance_(nullptr);

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
