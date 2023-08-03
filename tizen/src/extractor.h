#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>

#include <string>

#include "options.h"
#include "utils.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;

extern const char *kInitialDelaySeconds;
extern const char *kBackOffPolicyType;
extern const char *kBackOffPolicyDelayMillis;
extern const char *kOutofQuotaPolicy;
extern const char *kExistingWorkpolicy;

extern const char *kNetworkType;
extern const char *kBatteryNotLow;
extern const char *kCharging;
extern const char *kDeviceidle;
extern const char *kStorageNotLow;

// NetworkType
extern const char *kConnected;
extern const char *kMetered;
extern const char *kNotRequired;
extern const char *kNotRoaming;
extern const char *kUnmetered;
extern const char *kTemporarilyUnmetered;

// ExistingWorkPolicy
extern const char *kReplace;
extern const char *kKeep;
extern const char *kAppend;
extern const char *kUpdate;

// BackOffPolicy
extern const char *kExponential;
extern const char *kLinear;

// OutofQuotaPolicy
extern const char *kRunAsNonExpectedWorkRequest;
extern const char *kDropWorkRequest;

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
