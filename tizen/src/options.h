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

struct Constraints {
    const NetworkType network_type;

    const bool requires_battery_not_low;

    const bool requires_charging;

    const bool requires_device_idle;  // Not supported

    const bool requires_storage_not_low;  // Not supported

    Constraints(NetworkType network_type, bool requires_battery_not_low = false,
                bool requires_charging = false,
                bool requires_device_idle = false,
                bool requires_storage_not_low = false)
        : network_type(network_type),
          requires_battery_not_low(requires_battery_not_low),
          requires_charging(requires_charging),
          requires_device_idle(requires_device_idle),
          requires_storage_not_low(requires_storage_not_low) {}
};

struct TaskType {
    TaskType(int32_t minimum_backoff_delay)
        : minimum_backoff_delay(minimum_backoff_delay){};
    int32_t minimum_backoff_delay;
    enum { kOneOff = 10000, kPeriodic = 15 * 60 * 1000 };
};

struct BackoffPolicyTaskConfig {
    BackoffPolicy backoff_policy;

    int32_t request_backoff_delay;
    int32_t min_backoff_mills;
    int32_t backoff_delay;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_
