#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_

#include <bundle.h>

#include <string>
#include <variant>

#include "extractor.h"
#include "log.h"
#include "options.h"

extern const char *kMethodNameKey;
extern const char *kInitialize;
extern const char *kRegisterOneOffTask;
extern const char *kRegisterPeriodicTask;
extern const char *kCancelTaskByUniqueName;
extern const char *kCancelTaskByTag;
extern const char *kCancelAllTasks;
extern const char *kUnknown;

extern const char *kOnResultSendMethod;
extern const char *kBackgroundChannelInitialized;

extern const char *kIsInDebugMode;
extern const char *kCallbackhandle;
extern const char *kFrequencySeconds;
extern const char *kCancelTaskTag;
extern const char *kCancelTaskUniqueName;

extern const char *kUniquename;
extern const char *kNameValue;
extern const char *kTag;

extern const char *kConstraintsBundle;
extern const char *kBackOffPolicyBundle;

extern const char *kPayload;
extern const char *kIsPeriodic;

struct JobInfo {
    JobInfo(bool is_debug_mode, std::string unique_name, std::string task_name,
            ExistingWorkPolicy existing_work_policy,
            int32_t initial_delay_seconds, Constraints constraints,
            BackoffPolicyTaskConfig backoff_policy,
            OutOfQuotaPolicy out_of_quota_policy, int32_t frequency_seconds = 0,
            std::string tag = "", std::string payload = "",
            bool is_periodic = false)
        : is_debug_mode(is_debug_mode),
          unique_name(unique_name),
          task_name(task_name),
          tag(tag),
          initial_delay_seconds(initial_delay_seconds),
          constraints(constraints),
          payload(payload),
          frequency_seconds(frequency_seconds),
          backoff_policy(backoff_policy),
          out_of_quota_policy(out_of_quota_policy),
          is_periodic(is_periodic){};

    bool is_debug_mode;
    std::string unique_name;
    std::string task_name;
    std::string tag;
    int32_t initial_delay_seconds;
    Constraints constraints;
    std::string payload;
    ExistingWorkPolicy existing_work_policy;
    BackoffPolicyTaskConfig backoff_policy;
    OutOfQuotaPolicy out_of_quota_policy;
    int32_t frequency_seconds;
    bool is_periodic;
};

bundle *AddJobInfoToBundle(bundle *bund, JobInfo &job_info);
JobInfo GetFromBundle(bundle *bund);

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_
