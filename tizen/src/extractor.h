#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>

#include <string>

#include "constants.h"
#include "options.h"
#include "tasks.h"
#include "utils.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;

ExistingWorkPolicy ExtractExistingWorkPolicyFromCall(const FlMethodCall &call) {
    std::string value;
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);
    bool succeed = GetValueFromEncodableMap<std::string>(
        &map, constants::keys::kExistingWorkpolicykey, value);
    if (succeed) {
        Upper(value);

        if (value == constants::values::kReplace) {
            return ExistingWorkPolicy::kReplace;
        } else if (value == constants::values::kKeep) {
            return ExistingWorkPolicy::kKeep;
        } else if (value == constants::values::kAppend) {
            return ExistingWorkPolicy::kAppend;
        } else if (value == constants::values::kUpdate) {  // TODO : check
                                                           // real value
            return ExistingWorkPolicy::kUpdate;
        }
    }
    return ExistingWorkPolicy::kKeep;
}

std::optional<BackoffPolicyTaskConfig> ExtractBackoffPolicyConfigFromCall(
    const FlMethodCall &call, TaskType task_type) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::string value;
    if (!GetValueFromEncodableMap(&map, constants::keys::kBackOffPolicyTypeKey,
                                  value)) {
        return std::nullopt;
    }

    BackoffPolicy backoff_policy;
    if (value == constants::values::kExponential) {
        backoff_policy = BackoffPolicy::kExponential;
    } else {
        backoff_policy = BackoffPolicy::kLinear;
    }

    int32_t requested_backoff_delay =
        GetOrNullFromEncodableMap<int32_t>(
            &map, constants::keys::kBackOffPolicyDelayMillisKey)
            .value_or(15 * 6 * 1000) /
        1000;
    int32_t minimum_backoff_delay = task_type.minimum_backoff_delay;

    BackoffPolicyTaskConfig ret;
    ret.backoff_policy = backoff_policy;
    ret.request_backoff_delay = requested_backoff_delay;
    ret.min_backoff_mills = minimum_backoff_delay;

    return ret;
}

std::optional<OutOfQuotaPolicy> ExtractOutOfQuotaPolicyFromCall(
    const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::optional<std::string> value = GetOrNullFromEncodableMap<std::string>(
        &map, constants::keys::kOutofQuotaPolicyKey);

    if (!value.has_value()) {
        return std::nullopt;
    }

    Upper(value.value());

    if (value.value() == constants::values::kRunAsNonExpectedWorkRequest) {
        return OutOfQuotaPolicy::kRunAsNonExpeditedWorkRequest;
    } else if (value.value() == constants::values::kDropWorkRequest) {
        return OutOfQuotaPolicy::kDropWorkRequest;
    }

    return std::nullopt;
}

NetworkType ExtractNetworkTypeFromCall(const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::optional<std::string> value = GetOrNullFromEncodableMap<std::string>(
        &map, constants::keys::kNetworkTypekey);
    if (!value.has_value()) {
        return NetworkType::kNotRequired;
    }

    Upper(value.value());

    if (value.value() == constants::values::kConnected) {
        return NetworkType::kConnected;
    } else if (value.value() == constants::values::kMetered) {
        return NetworkType::kMetered;
    } else if (value.value() == constants::values::kNotRequired) {
        return NetworkType::kNotRequired;
    } else if (value.value() == constants::values::kNotRoaming) {
        return NetworkType::kNotRoaming;
    } else if (value.value() == constants::values::kUnmetered) {
        return NetworkType::kUnmetered;
    } else if (value.value() == constants::values::kTemporarilyUnmetered) {
        return NetworkType::kTemporarilyUnmetered;
    }

    return NetworkType::kNotRequired;
}

Constraints ExtractConstraintConfigFromCall(const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    NetworkType requested_network_type = ExtractNetworkTypeFromCall(call);
    bool requires_battery_not_low =
        GetOrNullFromEncodableMap<bool>(&map,
                                        constants::keys::kBatteryNotLowKey)
            .value_or(false);
    bool requires_charging =
        GetOrNullFromEncodableMap<bool>(&map, constants::keys::kChargingKey)
            .value_or(false);
    bool requires_device_idle =
        GetOrNullFromEncodableMap<bool>(&map, constants::keys::kDeviceidlekey)
            .value_or(false);
    bool requires_storage_not_low =
        GetOrNullFromEncodableMap<bool>(&map,
                                        constants::keys::kStorageNotLowKey)
            .value_or(false);

    return Constraints(requested_network_type, requires_battery_not_low,
                       requires_charging, requires_storage_not_low);
}

#endif
