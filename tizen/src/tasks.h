#ifndef FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_

#include <optional>
#include <string>
#include <variant>

#include "options.h"

class InitializeTask {
   public:
    InitializeTask(int32_t callBackDispathcerHandlerKey, bool isInDebugMode)
        : callBackDispathcerHandlerKey(callBackDispathcerHandlerKey),
          isInDebugMode(isInDebugMode){};

    int32_t callBackDispathcerHandlerKey = 0;
    bool isInDebugMode = false;
};

class RegisterTask {
   public:
    RegisterTask(bool isInDebugMode, std::string uniquename,
                 std::string taskname, std::optional<std::string> tag,
                 int32_t initialDelaySeconds,
                 std::optional<Constraints> constraintsConfig,
                 std::optional<std::string> payload)
        : is_in_debug_mode_(isInDebugMode),
          unique_name_(uniquename),
          task_name_(taskname),
          tag_(tag),
          initial_delay_seconds_(initialDelaySeconds),
          constraints_config_(constraintsConfig),
          payload_(payload){};

    bool is_in_debug_mode_;
    std::string unique_name_;
    std::string task_name_;
    std::optional<std::string> tag_;
    int32_t initial_delay_seconds_;
    std::optional<Constraints> constraints_config_;
    std::optional<std::string> payload_;
};

class OneoffTask : public RegisterTask {
   public:
    ExistingWorkPolicy existing_work_policy_;
    std::optional<BackoffPolicyTaskConfig> backoff_policy_config_;
    std::optional<OutOfQuotaPolicy> out_of_quota_policy_;

    OneoffTask(bool isInDebugMode, std::string uniqueName, std::string taskName,
               ExistingWorkPolicy existingWorkPolicy,
               int32_t initialDelaySeconds, Constraints constraintsConfig,
               std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig,
               std::optional<OutOfQuotaPolicy> outOfQuotaPolicy,
               std::optional<std::string> tag = std::nullopt,
               std::optional<std::string> payload = std::nullopt)
        : RegisterTask(isInDebugMode, uniqueName, taskName, tag,
                       initialDelaySeconds, constraintsConfig, payload),
          backoff_policy_config_(backoffPolicyConfig),
          out_of_quota_policy_(outOfQuotaPolicy){};
};

class PeriodicTask : public RegisterTask {
   public:
    PeriodicTask(bool isInDebugMode, std::string uniqueName,
                 std::string taskName, ExistingWorkPolicy existingWorkPolicy,
                 int32_t frequencyInSeconds, int32_t initialDelaySeconds,
                 Constraints constraintsConfig,
                 std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig,
                 std::optional<OutOfQuotaPolicy> outOfQuotaPolicy,
                 std::optional<std::string> tag = std::nullopt,
                 std::optional<std::string> payload = std::nullopt)
        : RegisterTask(isInDebugMode, uniqueName, taskName, tag,
                       initialDelaySeconds, constraintsConfig, payload),
          frequency_in_seconds_(frequencyInSeconds),
          backoff_policy_config_(backoffPolicyConfig),
          out_of_quota_policy_(outOfQuotaPolicy){};

    ExistingWorkPolicy existing_work_policy_;
    std::optional<BackoffPolicyTaskConfig> backoff_policy_config_;
    std::optional<OutOfQuotaPolicy> out_of_quota_policy_;
    int32_t frequency_in_seconds_;
};

class CancelTask {};

class CancelByTagTask : public CancelTask {
   public:
    CancelByTagTask(std::string tag) : tag_(tag){};
    std::string tag_;
};

class CancelByUniqueNameTask : public CancelTask {
   public:
    CancelByUniqueNameTask(std::string uniqueName) : uniqueName_(uniqueName){};
    std::string uniqueName_;
};

class UnknownTask {};

class FailedTask  {
   public:
    std::string code_;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
