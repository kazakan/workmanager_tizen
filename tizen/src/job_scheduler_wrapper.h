#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_

#include <job_scheduler.h>

#include <memory>

#include "log.h"
#include "options.h"
#include "tasks.h"

class JobScheduler {
   public:
    JobScheduler(JobScheduler const&) = delete;
    JobScheduler& operator=(JobScheduler const&) = delete;

    static JobScheduler& instance() {
        static JobScheduler instance;
        return instance;
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
        int err = job_info_create(&job_info);
        if (err) {
            LOG_ERROR("Failed to create job info: %s", get_error_message(err));
            return;
        }

        if (isPeriodic) {
            err = job_info_set_periodic(job_info,
                                        task.frequency_in_seconds.value_or(1));
            if (err) {
                LOG_ERROR("Failed to set job info periodic: %s",
                          get_error_message(err));
                return;
            }
            err = job_info_set_persistent(job_info, true);
            if (err) {
                LOG_ERROR("Failed to set job info persistent: %s",
                          get_error_message(err));
                return;
            }
        } else {
            err = job_info_set_once(job_info, true);
            if (err) {
                LOG_ERROR("Failed to set job info once: %s",
                          get_error_message(err));
                return;
            }
        }

        err = job_scheduler_schedule(job_info, task.unique_name.c_str());
        if (err) {
            LOG_ERROR("Failed to destroy job info: %s", get_error_message(err));
            return;
        }
        job_info_destroy(job_info);
    }

    void CancelByTag(const std::string& task) {
        // TODO : implement
    }

    void CancelByUniqueName(const std::string& name) {
        int err = job_scheduler_cancel(name.c_str());
        if (err) {
            LOG_ERROR("Failed to cancel job with name %s: %s", name.c_str(),
                      get_error_message(err));
        }
    }

    void CancelAll() { job_scheduler_cancel_all(); }

    job_service_h SetCallback(const std::string job_name,
                              job_service_callback_s& callback,
                              void* user_data) {
        job_service_h service = nullptr;
        int err = job_scheduler_service_add(job_name.c_str(), &callback,
                                            user_data, &service);
        if (err) {
            LOG_ERROR("Failed to add service to job: %s",
                      get_error_message(err));
            return nullptr;
        }
        return service;
    }

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
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
