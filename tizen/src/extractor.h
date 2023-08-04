#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>

#include <string>

#include "options.h"
#include "utils.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;

constexpr char *kInitialDelaySeconds = "initialDelaySeconds";
constexpr char *kBackOffPolicyType = "backoffPolicyType";
constexpr char *kBackOffPolicyDelayMillis = "backoffDelayInMilliseconds";
constexpr char *kOutofQuotaPolicy = "outOfQuotaPolicy";
constexpr char *kExistingWorkpolicy = "existingWorkPolicy";

constexpr char *kNetworkType = "networkType";
constexpr char *kBatteryNotLow = "requiresBatteryNotLow";
constexpr char *kCharging = "requiresCharging";
constexpr char *kDeviceidle = "requiresDeviceIdle";
constexpr char *kStorageNotLow = "requiresStorageNotLow";

// NetworkType
constexpr char *kConnected = "connectd";
constexpr char *kMetered = "metered";
constexpr char *kNotRequired = "not_required";
constexpr char *kNotRoaming = "not_roaming";
constexpr char *kUnmetered = "unmetered";
constexpr char *kTemporarilyUnmetered = "temporarily_unmetered";

// ExistingWorkPolicy
constexpr char *kReplace = "replace";
constexpr char *kKeep = "keep";
constexpr char *kAppend = "append";
constexpr char *kUpdate = "update";

// BackOffPolicy
constexpr char *kExponential = "exponential";
constexpr char *kLinear = "linear";

// OutofQuotaPolicy
constexpr char *kRunAsNonExpectedWorkRequest =
    "run_as_non_expedited_work_request";
constexpr char *kDropWorkRequest = "drop_work_request";

ExistingWorkPolicy StringToExistingWorkPolicy(const std::string &str);

BackoffPolicy StringToBackoffPolicy(const std::string &str);

OutOfQuotaPolicy StringToOutOfQuotaPolicy(const std::string &str);

NetworkType StringToNetworkType(const std::string &str);

ExistingWorkPolicy ExtractExistingWorkPolicyFromMap(
    const flutter::EncodableMap &map);

BackoffPolicyTaskConfig ExtractBackoffPolicyConfigFromMap(
    const flutter::EncodableMap &map, int32_t minimum_backoff_delay);

OutOfQuotaPolicy ExtractOutOfQuotaPolicyFromMap(
    const flutter::EncodableMap &map);

NetworkType ExtractNetworkTypeFromMap(const flutter::EncodableMap &args);

Constraints ExtractConstraintConfigFromMap(const flutter::EncodableMap &map);

#endif
