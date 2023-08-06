#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_

#include <bundle.h>

#include <string>
#include <variant>

#include "extractor.h"
#include "log.h"
#include "options.h"

constexpr const char* kMethodNameKey = "methodName";
constexpr const char* kInitialize = "initialize";
constexpr const char* kRegisterOneOffTask = "registerOneOffTask";
constexpr const char* kRegisterPeriodicTask = "registerPeriodicTask";
constexpr const char* kCancelTaskByUniqueName = "cancelTaskByUniqueName";
constexpr const char* kCancelTaskByTag = "cancelTaskBytag";
constexpr const char* kCancelAllTasks = "cancelAllTasks";
constexpr const char* kUnknown = "unknown";

constexpr const char* kOnResultSendMethod = "onResultSend";
constexpr const char* kBackgroundChannelInitialized = "backgroundChannelInitialized";

constexpr const char* kIsInDebugMode = "isInDebugMode";
constexpr const char* kCallbackhandle = "callbackHandle";
constexpr const char* kFrequencySeconds = "frequency";
constexpr const char* kCancelTaskTag = "tag";
constexpr const char* kCancelTaskUniqueName = "uniqueName";

constexpr const char* kUniquename = "uniqueName";
constexpr const char* kNameValue = "taskName";
constexpr const char* kTag = "tag";

constexpr const char* kConstraintsBundle = "constraintsBundle";
constexpr const char* kBackOffPolicyBundle = "backoffPolicyBundle";

constexpr const char* kPayload = "inputData";
constexpr const char* kIsPeriodic = "isPeriodic";

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

void AddJobInfoToBundle(bundle *bund, const JobInfo &job_info);
JobInfo GetFromBundle(bundle *bund);

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_
