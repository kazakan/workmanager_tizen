#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_INFO_H_

#include <bundle.h>

#include <string>
#include <variant>

#include "extractor.h"
#include "log.h"
#include "options.h"

constexpr char *kMethodNameKey = "methodName";
constexpr char *kInitialize = "initialize";
constexpr char *kRegisterOneOffTask = "registerOneOffTask";
constexpr char *kRegisterPeriodicTask = "registerPeriodicTask";
constexpr char *kCancelTaskByUniqueName = "cancelTaskByUniqueName";
constexpr char *kCancelTaskByTag = "cancelTaskBytag";
constexpr char *kCancelAllTasks = "cancelAllTasks";
constexpr char *kUnknown = "unknown";

constexpr char *kOnResultSendMethod = "onResultSend";
constexpr char *kBackgroundChannelInitialized = "backgroundChannelInitialized";

constexpr char *kIsInDebugMode = "isInDebugMode";
constexpr char *kCallbackhandle = "callbackHandle";
constexpr char *kFrequencySeconds = "frequency";
constexpr char *kCancelTaskTag = "tag";
constexpr char *kCancelTaskUniqueName = "uniqueName";

constexpr char *kUniquename = "uniqueName";
constexpr char *kNameValue = "taskName";
constexpr char *kTag = "tag";

constexpr char *kConstraintsBundle = "constraintsBundle";
constexpr char *kBackOffPolicyBundle = "backoffPolicyBundle";

constexpr char *kPayload = "inputData";
constexpr char *kIsPeriodic = "isPeriodic";

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
