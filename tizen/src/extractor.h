#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>

#include <string>

#include "options.h"
#include "tasks.h"
#include "utils.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;

const char *kInitialDelaySecondsKey = "initialDelaySeconds";
const char *kBackOffPolicyTypeKey = "backoffPolicyType";
const char *kBackOffPolicyDelayMillisKey = "backoffDelayInMilliseconds";
const char *kOutofQuotaPolicyKey = "outOfQuotaPolicy";
const char *kExistingWorkpolicykey = "existingWorkPolicy";

const char *kNetworkTypekey = "networkType";
const char *kBatteryNotLowKey = "requiresBatteryNotLow";
const char *kChargingKey = "requiresCharging";
const char *kDeviceidlekey = "requiresDeviceIdle";
const char *kStorageNotLowKey = "requiresStorageNotLow";

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

ExistingWorkPolicy ExtractExistingWorkPolicyFromMap(
    const flutter::EncodableMap &map) {
    std::string value;
    bool succeed = GetValueFromEncodableMap<std::string>(
        &map, kExistingWorkpolicykey, value);
    if (succeed) {
        if (value == kReplace) {
            return ExistingWorkPolicy::kReplace;
        } else if (value == kKeep) {
            return ExistingWorkPolicy::kKeep;
        } else if (value == kAppend) {
            return ExistingWorkPolicy::kAppend;
        } else if (value == kUpdate) {
            return ExistingWorkPolicy::kUpdate;
        }
    }
    return ExistingWorkPolicy::kKeep;
}

std::optional<BackoffPolicyTaskConfig> ExtractBackoffPolicyConfigFromMap(
    const flutter::EncodableMap &map, TaskType task_type) {
    std::string value;
    if (!GetValueFromEncodableMap(&map, kBackOffPolicyTypeKey, value)) {
        return std::nullopt;
    }

    BackoffPolicy backoff_policy;
    if (value == kExponential) {
        backoff_policy = BackoffPolicy::kExponential;
    } else {
        backoff_policy = BackoffPolicy::kLinear;
    }

    int32_t requested_backoff_delay =
        GetOrNullFromEncodableMap<int32_t>(&map, kBackOffPolicyDelayMillisKey)
            .value_or(15 * 6 * 1000) /
        1000;
    int32_t minimum_backoff_delay = task_type.minimum_backoff_delay;

    BackoffPolicyTaskConfig ret;
    ret.backoff_policy = backoff_policy;
    ret.request_backoff_delay = requested_backoff_delay;
    ret.min_backoff_mills = minimum_backoff_delay;

    return ret;
}

std::optional<OutOfQuotaPolicy> ExtractOutOfQuotaPolicyFromMap(
    const flutter::EncodableMap &map) {
    std::optional<std::string> value =
        GetOrNullFromEncodableMap<std::string>(&map, kOutofQuotaPolicyKey);

    if (!value.has_value()) {
        return std::nullopt;
    }

    if (value.value() == kRunAsNonExpectedWorkRequest) {
        return OutOfQuotaPolicy::kRunAsNonExpeditedWorkRequest;
    } else if (value.value() == kDropWorkRequest) {
        return OutOfQuotaPolicy::kDropWorkRequest;
    }

    return std::nullopt;
}

NetworkType ExtractNetworkTypeFromMap(const flutter::EncodableMap &args) {
    std::optional<std::string> value =
        GetOrNullFromEncodableMap<std::string>(&args, kNetworkTypekey);
    if (!value.has_value()) {
        return NetworkType::kNotRequired;
    }

    if (value.value() == kConnected) {
        return NetworkType::kConnected;
    } else if (value.value() == kMetered) {
        return NetworkType::kMetered;
    } else if (value.value() == kNotRequired) {
        return NetworkType::kNotRequired;
    } else if (value.value() == kNotRoaming) {
        return NetworkType::kNotRoaming;
    } else if (value.value() == kUnmetered) {
        return NetworkType::kUnmetered;
    } else if (value.value() == kTemporarilyUnmetered) {
        return NetworkType::kTemporarilyUnmetered;
    }

    return NetworkType::kNotRequired;
}

Constraints ExtractConstraintConfigFromMap(const flutter::EncodableMap &map) {
    NetworkType network_type = ExtractNetworkTypeFromMap(map);
    bool battery_not_low =
        GetOrNullFromEncodableMap<bool>(&map, kBatteryNotLowKey)
            .value_or(false);
    bool charging =
        GetOrNullFromEncodableMap<bool>(&map, kChargingKey).value_or(false);
    bool device_idle =
        GetOrNullFromEncodableMap<bool>(&map, kDeviceidlekey).value_or(false);
    bool storage_not_low =
        GetOrNullFromEncodableMap<bool>(&map, kStorageNotLowKey)
            .value_or(false);

    return Constraints(network_type, battery_not_low, charging,
                       storage_not_low);
}

#endif
