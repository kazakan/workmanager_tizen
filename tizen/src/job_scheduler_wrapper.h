#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_

#include <job_scheduler.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "job.h"
#include "options.h"

extern const char* kPayloadPreferencePrefix;
extern const char* kTaskInfoPreferencePrefix;

class JobScheduler {
   public:
    JobScheduler(JobScheduler const&) = delete;
    JobScheduler& operator=(JobScheduler const&) = delete;

    static JobScheduler& instance() {
        static JobScheduler instance;
        return instance;
    }

    int SetJobConstraints(job_info_h job_handle,
                          const Constraints& constraints);

    void RegisterJob(const JobInfo& job_info,
                     job_service_callback_s* callback = nullptr);

    void CancelByTag(const std::string& task);

    void CancelByUniqueName(const std::string& name);

    void CancelAll();

    job_service_h SetCallback(const char* job_name,
                              job_service_callback_s* callback);

    std::vector<std::string> GetAllJobs();

   private:
    std::unordered_map<std::string, job_service_h> job_service_handles_;

    JobScheduler();
    ~JobScheduler() = default;

    void SavePayload(const std::string& job_name, const std::string& payload);
    void SaveJobInfo(const std::string& job_name, const JobInfo job_info);

    std::string GetPayloadKey(const std::string& job_name);
    std::string GetJobInfoKey(const std::string& job_name);
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
