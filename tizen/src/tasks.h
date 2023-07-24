#ifndef FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_

#include <optional>
#include <string>
#include <variant>

#include "options.h"

class WorkManagerCall {
   public:
    WorkManagerCall(){};
};

class InitializeTask : public WorkManagerCall {
   public:
    InitializeTask(int32_t callBackDispathcerHandlerKey, bool isInDebugMode)
        : callBackDispathcerHandlerKey(callBackDispathcerHandlerKey),
          isInDebugMode(isInDebugMode){};

    int32_t callBackDispathcerHandlerKey = 0;
    bool isInDebugMode = false;
};

class RegisterTask : public WorkManagerCall {
   public:
    RegisterTask(bool isInDebugMode, std::string uniquename,
                 std::string taskname, std::optional<std::string> tag,
                 int32_t initialDelaySeconds,
                 std::optional<Constraints> constraintsConfig,
                 std::optional<std::string> payload)
        : isInDebugMode(isInDebugMode),
          uniquename(uniquename),
          taskname(taskname),
          tag(tag),
          initialDelaySeconds(initialDelaySeconds),
          constraintsConfig(constraintsConfig),
          payload(payload){};

    bool isInDebugMode;
    std::string uniquename;
    std::string taskname;
    std::optional<std::string> tag;
    int32_t initialDelaySeconds;
    std::optional<Constraints> constraintsConfig;
    std::optional<std::string> payload;
};

class OneoffTask : public RegisterTask {
   public:
    ExistingWorkPolicy existingWorkPolicy;
    std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig;
    std::optional<OutOfQuotaPolicy> outOfQuotaPolicy;

    OneoffTask(bool isInDebugMode, std::string uniqueName, std::string taskName,
               ExistingWorkPolicy existingWorkPolicy,
               int32_t initialDelaySeconds, Constraints constraintsConfig,
               std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig,
               std::optional<OutOfQuotaPolicy> outOfQuotaPolicy,
               std::optional<std::string> tag = std::nullopt,
               std::optional<std::string> payload = std::nullopt)
        : RegisterTask(isInDebugMode, uniqueName, taskName, tag,
                       initialDelaySeconds, constraintsConfig, payload),
          backoffPolicyConfig(backoffPolicyConfig),
          outOfQuotaPolicy(outOfQuotaPolicy){};
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
          frequencyInSeconds(frequencyInSeconds),
          backoffPolicyConfig(backoffPolicyConfig),
          outOfQuotaPolicy(outOfQuotaPolicy){};

    ExistingWorkPolicy existingWorkPolicy;
    std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig;
    std::optional<OutOfQuotaPolicy> outOfQuotaPolicy;
    int32_t frequencyInSeconds;
};

class CancelTask : public WorkManagerCall {};

class CancelByTagTask : public CancelTask {
   public:
    CancelByTagTask(std::string tag) : tag(tag){};
    std::string tag;
};

class CancelByUniqueNameTask : public CancelTask {
   public:
    CancelByUniqueNameTask(std::string uniqueName) : uniqueName(uniqueName){};
    std::string uniqueName;
};

class UnknownTask : public WorkManagerCall {};

class FailedTask : public WorkManagerCall {
   public:
    std::string code;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_TASKS_H_