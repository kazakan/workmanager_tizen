#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>
#include <flutter/method_channel.h>

#include <string>

#include "constants.h"
#include "fltypes.h"
#include "options.h"
#include "tasks.h"
#include "utils.h"

class PossibleWorkManagerCall {
   public:
    std::string rawMethodName;

    static PossibleWorkManagerCall fromRawMethodName(
        const std::string &methodName);
};

ExistingWorkPolicy extractExistingWorkPolicyFromCall(const FlMethodCall &call) {
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

std::optional<BackoffPolicyTaskConfig> extractBackoffPolicyConfigFromCall(
    const FlMethodCall &call, TaskType taskType) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::string value;
    if (!GetValueFromEncodableMap(&map, constants::keys::kbackOffPolicyTypeKey,
                                  value)) {
        return std::nullopt;
    }

    BackoffPolicy backoffPolicy;

    if (value == constants::values::kExponential) {
        backoffPolicy = BackoffPolicy::kExponential;
    } else {
        backoffPolicy = BackoffPolicy::kLinear;
    }

    int32_t requestedBackoffDelay =
        GetOrNullFromEncodableMap<int32_t>(
            &map, constants::keys::kBackOffPolicyDelayMillisKey)
            .value_or(15 * 6 * 1000) /
        1000;
    int32_t minimumBackOffDelay = taskType.minimum_backoff_delay_;

    BackoffPolicyTaskConfig ret;
    ret.backoff_policy_ = backoffPolicy;
    ret.request_backoff_delay_ = requestedBackoffDelay;
    ret.min_backoff_mills_ = minimumBackOffDelay;

    return ret;
}

std::optional<OutOfQuotaPolicy> extractOutOfQuotaPolicyFromCall(
    const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::optional<std::string> value = GetOrNullFromEncodableMap<std::string>(
        &map, constants::keys::kOutofQuotaPolicyKey);

    if (!value.has_value()) return std::nullopt;

    Upper(value.value());

    if (value.value() == constants::values::kRunAsNonExpectedWorkRequest) {
        return OutOfQuotaPolicy::kRunAsNonExpeditedWorkRequest;
    } else if (value.value() == constants::values::kDropWorkRequest) {
        return OutOfQuotaPolicy::kDropWorkRequest;
    }

    return std::nullopt;
}

NetworkType extractNetworkTypeFromCall(const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::optional<std::string> value = GetOrNullFromEncodableMap<std::string>(
        &map, constants::keys::kNetworkTypekey);
    if (!value.has_value()) return NetworkType::kNotRequired;

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

Constraints extractConstraintConfigFromCall(const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    NetworkType requestedNetworktype = extractNetworkTypeFromCall(call);
    bool requiresBatteryNotLow = GetOrNullFromEncodableMap<bool>(
                                     &map, constants::keys::kBatteryNotLowKey)
                                     .value_or(false);
    bool requiresCharging =
        GetOrNullFromEncodableMap<bool>(&map, constants::keys::kChargingKey)
            .value_or(false);
    bool requiresDeviceidle =
        GetOrNullFromEncodableMap<bool>(&map, constants::keys::kDeviceidlekey)
            .value_or(false);
    bool requiresStorageNotLow = GetOrNullFromEncodableMap<bool>(
                                     &map, constants::keys::kStorageNotLowKey)
                                     .value_or(false);

    return Constraints(requestedNetworktype, requiresBatteryNotLow,
                       requiresCharging, requiresStorageNotLow);
}

#endif
