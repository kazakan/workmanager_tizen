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

static ExistingWorkPolicy extractExistingWorkPolicyFromCall(
    const FlMethodCall &call) {
    std::string value;
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);
    bool succeed = GetValueFromEncodableMap<std::string>(
        &map, kExistingWorkpolicykey, value);
    if (succeed) {
        upper(value);

        if (value == kReplace) {
            return ExistingWorkPolicy::replace;
        } else if (value == kKeep) {
            return ExistingWorkPolicy::keep;
        } else if (value == kAppend) {
            return ExistingWorkPolicy::append;
        } else if (value == kUpdate) {  // TODO : check
                                        // real value
            return ExistingWorkPolicy::update;
        }
    }
    return ExistingWorkPolicy::keep;
}

static std::optional<BackoffPolicyTaskConfig>
extractBackoffPolicyConfigFromCall(const FlMethodCall &call,
                                   TaskType taskType) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::string value;
    if (!GetValueFromEncodableMap(&map, kbackOffPolicyTypeKey, value)) {
        return std::nullopt;
    }

    BackoffPolicy backoffPolicy;

    if (value == kExponential) {
        backoffPolicy = BackoffPolicy::exponential;
    } else {
        backoffPolicy = BackoffPolicy::linear;
    }

    int32_t requestedBackoffDelay =
        GetOrNullFromEncodableMap<int32_t>(&map, kBackOffPolicyDelayMillisKey)
            .value_or(15 * 6 * 1000) /
        1000;
    int32_t minimumBackOffDelay = taskType.minimumbackOffDelay;

    BackoffPolicyTaskConfig ret;
    ret.backoffPolicy = backoffPolicy;
    ret.requestbackoffDelay = requestedBackoffDelay;
    ret.minbackoffMills = minimumBackOffDelay;

    return ret;
}

static std::optional<OutOfQuotaPolicy> extractOutOfQuotaPolicyFromCall(
    const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::optional<std::string> value =
        GetOrNullFromEncodableMap<std::string>(&map, kOutofQuotaPolicyKey);

    if (!value.has_value()) return std::nullopt;

    upper(value.value());

    if (value.value() == kRunAsNonExpectedWorkRequest) {
        return OutOfQuotaPolicy::run_as_non_expedited_work_request;
    } else if (value.value() == kDropWorkRequest) {
        return OutOfQuotaPolicy::drop_work_request;
    }

    return std::nullopt;
}

static NetworkType extractNetworkTypeFromCall(const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    std::optional<std::string> value =
        GetOrNullFromEncodableMap<std::string>(&map, kNetworkTypekey);
    if (!value.has_value()) return NetworkType::not_required;

    upper(value.value());

    if (value.value() == kConnected) {
        return NetworkType::connected;
    } else if (value.value() == kMetered) {
        return NetworkType::metered;
    } else if (value.value() == kNotRequired) {
        return NetworkType::not_required;
    } else if (value.value() == kNotRoaming) {
        return NetworkType::not_roaming;
    } else if (value.value() == kUnmetered) {
        return NetworkType::unmetered;
    } else if (value.value() == kTemporarilyUnmetered) {
        return NetworkType::temporarily_unmetered;
    }

    return NetworkType::not_required;
}

static Constraints extractConstraintConfigFromCall(const FlMethodCall &call) {
    const auto &args = *call.arguments();
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);

    NetworkType requestedNetworktype = extractNetworkTypeFromCall(call);
    bool requiresBatteryNotLow =
        GetOrNullFromEncodableMap<bool>(&map, kBatteryNotLowKey)
            .value_or(false);
    bool requiresCharging =
        GetOrNullFromEncodableMap<bool>(&map, kChargingKey).value_or(false);
    bool requiresDeviceidle =
        GetOrNullFromEncodableMap<bool>(&map, kDeviceidlekey).value_or(false);
    bool requiresStorageNotLow =
        GetOrNullFromEncodableMap<bool>(&map, kStorageNotLowKey)
            .value_or(false);

    return Constraints(requestedNetworktype, requiresBatteryNotLow,
                       requiresCharging, requiresStorageNotLow);
}

#endif