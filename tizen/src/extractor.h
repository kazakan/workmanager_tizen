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
const char *kConnected = "CONNECTED";
const char *kMetered = "METERED";
const char *kNotRequired = "NOT_REQUIRED";
const char *kNotRoaming = "NOT_ROAMING";
const char *kUnmetered = "UNMETERED";
const char *kTemporarilyUnmetered = "TEMPORARILY_UNMETERED";

// ExistingWorkPolicy
const char *kReplace = "REPLACE";
const char *kKeep = "KEEP";
const char *kAppend = "APPEND";
const char *kUpdate = "UPDATE";

// BackOffPolicy
const char *kExponential = "EXPONENTIAL";
const char *kLinear = "LINEAR";

// OutofQuotaPolicy
const char *kRunAsNonExpectedWorkRequest = "RUN_AS_NON_EXPEDITED_WORK_REQUEST";
const char *kDropWorkRequest = "DROP_WORK_REQUEST";

ExistingWorkPolicy ExtractExistingWorkPolicyFromMap(
    const flutter::EncodableMap &map) {
    std::string value;
    bool succeed = GetValueFromEncodableMap<std::string>(
        &map, kExistingWorkpolicykey, value);
    if (succeed) {
        Upper(value);

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

    Upper(value.value());

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

    Upper(value.value());

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
    NetworkType requested_network_type = ExtractNetworkTypeFromMap(map);
    bool requires_battery_not_low =
        GetOrNullFromEncodableMap<bool>(&map, kBatteryNotLowKey)
            .value_or(false);
    bool requires_charging =
        GetOrNullFromEncodableMap<bool>(&map, kChargingKey).value_or(false);
    bool requires_device_idle =
        GetOrNullFromEncodableMap<bool>(&map, kDeviceidlekey).value_or(false);
    bool requires_storage_not_low =
        GetOrNullFromEncodableMap<bool>(&map, kStorageNotLowKey)
            .value_or(false);

    return Constraints(requested_network_type, requires_battery_not_low,
                       requires_charging, requires_storage_not_low);
}

#endif
