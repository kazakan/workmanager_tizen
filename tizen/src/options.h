#ifndef FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_

enum class ExistingWorkPolicy { kReplace, kKeep, kAppend, kUpdate };

enum class NetworkType {
    kConnected,
    kMetered,
    kNotRequired,
    kNotRoaming,
    kUnmetered,
    kTemporarilyUnmetered,
};

enum class OutOfQuotaPolicy {
    kRunAsNonExpeditedWorkRequest,
    kDropWorkRequest,
};

enum class BackoffPolicy { kExponential, kLinear };

class Constraints {
   public:
    const NetworkType network_type_;

    const bool requires_battery_not_low_;

    const bool requires_charging_;

    const bool requires_device_idle_;  // Not supported

    const bool requires_storage_not_low_;  // Not supported

    Constraints(NetworkType networkType, bool requiresBatteryNotLow = false,
                bool requiresCharging = false, bool requiresDeviceIdle = false,
                bool requiresStorageNotLow = false)
        : network_type_(networkType),
          requires_battery_not_low_(requiresBatteryNotLow),
          requires_charging_(requiresCharging),
          requires_device_idle_(requiresDeviceIdle),
          requires_storage_not_low_(requiresStorageNotLow) {}
};

class TaskType {
   public:
    TaskType(int32_t minimumbackOffDelay)
        : minimum_backoff_delay_(minimumbackOffDelay){};
    int32_t minimum_backoff_delay_;
    enum { kOneOff = 10000, kPeriodic = 15 * 60 * 1000 };
};

class BackoffPolicyTaskConfig {
   public:
    BackoffPolicy backoff_policy_;

    int32_t request_backoff_delay_;
    int32_t min_backoff_mills_;
    int32_t backoff_delay_;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_
