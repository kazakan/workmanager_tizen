#ifndef FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_

enum class ExistingWorkPolicy { replace, keep, append, update };

enum class NetworkType {
    connected,
    metered,
    not_required,
    not_roaming,
    unmetered,
    temporarily_unmetered,
};

enum class OutOfQuotaPolicy {
    run_as_non_expedited_work_request,
    drop_work_request,
};

enum class BackoffPolicy { exponential, linear };

class Constraints {
   public:
    const NetworkType networkType;

    const bool requiresBatteryNotLow;

    const bool requiresCharging;

    const bool requiresDeviceIdle;  // Not supported

    const bool requiresStorageNotLow;  // Not supported

    Constraints(NetworkType networkType, bool requiresBatteryNotLow = false,
                bool requiresCharging = false, bool requiresDeviceIdle = false,
                bool requiresStorageNotLow = false)
        : networkType(networkType),
          requiresBatteryNotLow(requiresBatteryNotLow),
          requiresCharging(requiresCharging),
          requiresDeviceIdle(requiresDeviceIdle),
          requiresStorageNotLow(requiresStorageNotLow) {}
};

class TaskType {
   public:
    TaskType(int32_t minimumbackOffDelay)
        : minimumbackOffDelay(minimumbackOffDelay){};
    int32_t minimumbackOffDelay;
    enum { ONE_OFF = 10000, PERIODIC = 15 * 60 * 1000 };
};

class BackoffPolicyTaskConfig {
   public:
    BackoffPolicy backoffPolicy;

    int32_t requestbackoffDelay;
    int32_t minbackoffMills;
    int32_t backoffDelay;
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_