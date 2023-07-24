#ifndef FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_

#include <optional>
#include <string>
#include <variant>

#include "options.h"

class InitializeTask {
   public:
    InitializeTask(int32_t callback_dispathcer_handler_key, bool is_in_debug_mode)
        : callback_dispathcer_handler_key_(callback_dispathcer_handler_key),
          is_in_debug_mode_(is_in_debug_mode){};

    int32_t callback_dispathcer_handler_key_ = 0;
    bool is_in_debug_mode_ = false;
};

class RegisterTask {
   public:
    RegisterTask(bool is_in_debug_mode, std::string unique_name,
                 std::string task_name, std::optional<std::string> tag,
                 int32_t initial_delay_seconds,
                 std::optional<Constraints> constraints_config,
                 std::optional<std::string> payload)
        : is_in_debug_mode_(is_in_debug_mode),
          unique_name_(unique_name),
          task_name_(task_name),
          tag_(tag),
          initial_delay_seconds_(initial_delay_seconds),
          constraints_config_(constraints_config),
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

    OneoffTask(bool is_in_debug_mode, std::string unique_name, std::string task_name,
               ExistingWorkPolicy existing_work_policy,
               int32_t initial_delay_seconds, Constraints constraints_config,
               std::optional<BackoffPolicyTaskConfig> backoff_policy_config,
               std::optional<OutOfQuotaPolicy> out_of_quota_policy,
               std::optional<std::string> tag = std::nullopt,
               std::optional<std::string> payload = std::nullopt)
        : RegisterTask(is_in_debug_mode, unique_name, task_name, tag,
                       initial_delay_seconds, constraints_config, payload),
          backoff_policy_config_(backoff_policy_config),
          out_of_quota_policy_(out_of_quota_policy){};
};

class PeriodicTask : public RegisterTask {
   public:
    PeriodicTask(bool is_in_debug_mode, std::string unique_name,
                 std::string task_name, ExistingWorkPolicy existing_work_policy,
                 int32_t frequency_in_seconds, int32_t initial_delay_seconds,
                 Constraints constraints_config,
                 std::optional<BackoffPolicyTaskConfig> backoff_policy_config,
                 std::optional<OutOfQuotaPolicy> out_of_quota_policy,
                 std::optional<std::string> tag = std::nullopt,
                 std::optional<std::string> payload = std::nullopt)
        : RegisterTask(is_in_debug_mode, unique_name, task_name, tag,
                       initial_delay_seconds, constraints_config, payload),
          frequency_in_seconds_(frequency_in_seconds),
          backoff_policy_config_(backoff_policy_config),
          out_of_quota_policy_(out_of_quota_policy){};

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
    CancelByUniqueNameTask(std::string unique_name) : uniqueName_(unique_name){};
    std::string uniqueName_;
};

class UnknownTask {};

class FailedTask  {
   public:
    std::string code_;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
