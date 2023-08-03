#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_

#include <job_scheduler.h>

#include <memory>
#include <vector>

#include "options.h"

extern const char* kPayloadPreferencePrefix;

class JobScheduler {
   public:
    JobScheduler(JobScheduler const&) = delete;
    JobScheduler& operator=(JobScheduler const&) = delete;

    static JobScheduler& instance() {
        static JobScheduler instance;
        return instance;
    }

    int SetJobConstraints(job_info_h job_info, const Constraints& constraints);

    void RegisterJob(const bool is_debug_mode, const std::string& unique_name,
                     const std::string& task_name,
                     const ExistingWorkPolicy existing_work_policy,
                     const int32_t initial_delay_seconds,
                     const Constraints& constraints_config,
                     const BackoffPolicyTaskConfig& backoff_policy_config,
                     const OutOfQuotaPolicy& out_of_quota_policy,
                     const bool isPeriodic = false,
                     const int32_t frequency_seconds = 0,
                     const std::string& tag = "",
                     const std::string& payload = "",
                     job_service_callback_s* callback = nullptr);

    void CancelByTag(const std::string& task);

    void CancelByUniqueName(const std::string& name);

    void CancelAll();

    job_service_h SetCallback(const char* job_name,
                              job_service_callback_s* callback);

    std::vector<std::string> GetAllJobs();

   private:
    JobScheduler();
    ~JobScheduler() = default;

    void SavePayload(const std::string& job_name, const std::string& payload);

    std::string GetPayloadKey(const std::string& job_name);
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
