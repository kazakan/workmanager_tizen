#ifndef FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_

#include <optional>
#include <string>
#include <variant>

#include "options.h"

struct InitializeTaskInfo {
    InitializeTaskInfo(int32_t callback_dispathcer_handler_key,
                       bool is_in_debug_mode)
        : callback_dispathcer_handler_key(callback_dispathcer_handler_key),
          is_in_debug_mode(is_in_debug_mode){};

    int32_t callback_dispathcer_handler_key = 0;
    bool is_in_debug_mode = false;
};

struct RegisterTaskInfo {
    RegisterTaskInfo(bool is_in_debug_mode, std::string unique_name,
                     std::string task_name,
                     ExistingWorkPolicy existing_work_policy,
                     int32_t initial_delay_seconds,
                     Constraints constraints_config,
                     BackoffPolicyTaskConfig backoff_policy_config,
                     OutOfQuotaPolicy out_of_quota_policy,
                     int32_t frequency_in_seconds = 0, std::string tag = "",
                     std::string payload = "",
                     bool is_periodic = false)
        : is_in_debug_mode(is_in_debug_mode),
          unique_name(unique_name),
          task_name(task_name),
          tag(tag),
          initial_delay_seconds(initial_delay_seconds),
          constraints_config(constraints_config),
          payload(payload),
          frequency_in_seconds(frequency_in_seconds),
          backoff_policy_config(backoff_policy_config),
          out_of_quota_policy(out_of_quota_policy),
          is_periodic(is_periodic){};

    bool is_in_debug_mode;
    std::string unique_name;
    std::string task_name;
    std::string tag;
    int32_t initial_delay_seconds;
    Constraints constraints_config;
    std::string payload;
    ExistingWorkPolicy existing_work_policy;
    BackoffPolicyTaskConfig backoff_policy_config;
    OutOfQuotaPolicy out_of_quota_policy;
    int32_t frequency_in_seconds;
    bool is_periodic;
};

struct FailedTaskInfo {
    std::string code;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
