#ifndef FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_

#include <optional>
#include <string>
#include <variant>

#include "options.h"

struct InitializeTask {
    InitializeTask(int32_t callback_dispathcer_handler_key,
                   bool is_in_debug_mode)
        : callback_dispathcer_handler_key(callback_dispathcer_handler_key),
          is_in_debug_mode(is_in_debug_mode){};

    int32_t callback_dispathcer_handler_key = 0;
    bool is_in_debug_mode = false;
};

struct RegisterTask {
    RegisterTask(bool is_in_debug_mode, std::string unique_name,
                 std::string task_name, std::optional<std::string> tag,
                 int32_t initial_delay_seconds,
                 std::optional<Constraints> constraints_config,
                 std::optional<std::string> payload)
        : is_in_debug_mode(is_in_debug_mode),
          unique_name(unique_name),
          task_name(task_name),
          tag(tag),
          initial_delay_seconds(initial_delay_seconds),
          constraints_config(constraints_config),
          payload(payload){};

    bool is_in_debug_mode;
    std::string unique_name;
    std::string task_name;
    std::optional<std::string> tag;
    int32_t initial_delay_seconds;
    std::optional<Constraints> constraints_config;
    std::optional<std::string> payload;
};

struct OneoffTask : public RegisterTask {
    OneoffTask(bool is_in_debug_mode, std::string unique_name,
               std::string task_name, ExistingWorkPolicy existing_work_policy,
               int32_t initial_delay_seconds, Constraints constraints_config,
               std::optional<BackoffPolicyTaskConfig> backoff_policy_config,
               std::optional<OutOfQuotaPolicy> out_of_quota_policy,
               std::optional<std::string> tag = std::nullopt,
               std::optional<std::string> payload = std::nullopt)
        : RegisterTask(is_in_debug_mode, unique_name, task_name, tag,
                       initial_delay_seconds, constraints_config, payload),
          backoff_policy_config(backoff_policy_config),
          out_of_quota_policy(out_of_quota_policy){};

    ExistingWorkPolicy existing_work_policy;
    std::optional<BackoffPolicyTaskConfig> backoff_policy_config;
    std::optional<OutOfQuotaPolicy> out_of_quota_policy;
};

struct PeriodicTask : public RegisterTask {
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
          frequency_in_seconds(frequency_in_seconds),
          backoff_policy_config(backoff_policy_config),
          out_of_quota_policy(out_of_quota_policy){};

    ExistingWorkPolicy existing_work_policy;
    std::optional<BackoffPolicyTaskConfig> backoff_policy_config;
    std::optional<OutOfQuotaPolicy> out_of_quota_policy;
    int32_t frequency_in_seconds;
};

struct CancelByTagTask {
    CancelByTagTask(std::string tag) : tag(tag){};
    std::string tag;
};

struct CancelByNameTask {
    CancelByNameTask(std::string name) : name(name){};
    std::string name;
};

struct FailedTask {
    std::string code;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
