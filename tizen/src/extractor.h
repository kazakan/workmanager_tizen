#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>

#include <string>

#include "options.h"
#include "utils.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;

const char *kInitialDelaySeconds = "initialDelaySeconds";
const char *kBackOffPolicyType = "backoffPolicyType";
const char *kBackOffPolicyDelayMillis = "backoffDelayInMilliseconds";
const char *kOutofQuotaPolicy = "outOfQuotaPolicy";
const char *kExistingWorkpolicy = "existingWorkPolicy";

const char *kNetworkType = "networkType";
const char *kBatteryNotLow = "requiresBatteryNotLow";
const char *kCharging = "requiresCharging";
const char *kDeviceidle = "requiresDeviceIdle";
const char *kStorageNotLow = "requiresStorageNotLow";

// NetworkType
const char *kConnected = "connectd";
const char *kMetered = "metered";
const char *kNotRequired = "not_required";
const char *kNotRoaming = "not_roaming";
const char *kUnmetered = "unmetered";
const char *kTemporarilyUnmetered = "temporarily_unmetered";

// ExistingWorkPolicy
const char *kReplace = "replace";
const char *kKeep = "keep";
const char *kAppend = "append";
const char *kUpdate = "update";

// BackOffPolicy
const char *kExponential = "exponential";
const char *kLinear = "linear";

// OutofQuotaPolicy
const char *kRunAsNonExpectedWorkRequest = "run_as_non_expedited_work_request";
const char *kDropWorkRequest = "drop_work_request";

ExistingWorkPolicy StringToExistingWorkPolicy(const std::string &str) {
    if (str == kReplace) {
        return ExistingWorkPolicy::kReplace;
    } else if (str == kAppend) {
        return ExistingWorkPolicy::kAppend;
    } else if (str == kUpdate) {
        return ExistingWorkPolicy::kUpdate;
    }
    return ExistingWorkPolicy::kKeep;
}

BackoffPolicy StringToBackoffPolicy(const std::string &str) {
    if (str == kExponential) {
        return BackoffPolicy::kExponential;
    }
    return BackoffPolicy::kLinear;
}

OutOfQuotaPolicy StringToOutOfQuotaPolicy(const std::string &str) {
    if (str == kRunAsNonExpectedWorkRequest) {
        return OutOfQuotaPolicy::kRunAsNonExpeditedWorkRequest;
    }
    return OutOfQuotaPolicy::kDropWorkRequest;
}

NetworkType StringToNetworkType(const std::string &str) {
    if (str == kConnected) {
        return NetworkType::kConnected;
    } else if (str == kMetered) {
        return NetworkType::kMetered;
    } else if (str == kNotRoaming) {
        return NetworkType::kNotRoaming;
    } else if (str == kUnmetered) {
        return NetworkType::kUnmetered;
    } else if (str == kTemporarilyUnmetered) {
        return NetworkType::kTemporarilyUnmetered;
    }
    return NetworkType::kNotRequired;
}

ExistingWorkPolicy ExtractExistingWorkPolicyFromMap(
    const flutter::EncodableMap &map) {
    std::string value;
    GetValueFromEncodableMap<std::string>(&map, kExistingWorkpolicy, value);
    return StringToExistingWorkPolicy(value);
}

BackoffPolicyTaskConfig ExtractBackoffPolicyConfigFromMap(
    const flutter::EncodableMap &map, int32_t minimum_backoff_delay) {
    std::string value;
    if (!GetValueFromEncodableMap(&map, kBackOffPolicyType, value)) {
        return {BackoffPolicy::kLinear, 15 * 60, minimum_backoff_delay, 0};
    }

    BackoffPolicy backoff_policy = StringToBackoffPolicy(value);

    int32_t requested_backoff_delay =
        GetOrNullFromEncodableMap<int32_t>(&map, kBackOffPolicyDelayMillis)
            .value_or(15 * 60 * 1000) /
        1000;

    BackoffPolicyTaskConfig ret;
    ret.backoff_policy = backoff_policy;
    ret.request_backoff_delay = requested_backoff_delay;
    ret.min_backoff_mills = minimum_backoff_delay;

    return ret;
}

OutOfQuotaPolicy ExtractOutOfQuotaPolicyFromMap(
    const flutter::EncodableMap &map) {
    std::string value =
        GetOrNullFromEncodableMap<std::string>(&map, kOutofQuotaPolicy)
            .value_or("");
    return StringToOutOfQuotaPolicy(value);
}

NetworkType ExtractNetworkTypeFromMap(const flutter::EncodableMap &args) {
    std::optional<std::string> value =
        GetOrNullFromEncodableMap<std::string>(&args, kNetworkType);
    if (!value.has_value()) {
        return NetworkType::kNotRequired;
    }

    return StringToNetworkType(value.value());
}

Constraints ExtractConstraintConfigFromMap(const flutter::EncodableMap &map) {
    NetworkType network_type = ExtractNetworkTypeFromMap(map);
    bool battery_not_low =
        GetOrNullFromEncodableMap<bool>(&map, kBatteryNotLow).value_or(false);
    bool charging =
        GetOrNullFromEncodableMap<bool>(&map, kCharging).value_or(false);
    bool device_idle =
        GetOrNullFromEncodableMap<bool>(&map, kDeviceidle).value_or(false);
    bool storage_not_low =
        GetOrNullFromEncodableMap<bool>(&map, kStorageNotLow).value_or(false);

    return Constraints(network_type, battery_not_low, charging,
                       storage_not_low);
}

#endif
