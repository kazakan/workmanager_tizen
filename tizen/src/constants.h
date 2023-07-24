#ifndef FLUTTER_PLUGIN_WORKMANAGER_CONSTANTS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_CONSTANTS_H_

namespace constants {

namespace methods {

const char *kInitialize = "initialize";
const char *kRegisterOneOffTask = "registerOneOffTask";
const char *kRegisterPeriodicTask = "registerPeriodicTask";
const char *kCancelTaskByUniqueName = "cancelTaskByUniqueName";
const char *kCancelTaskByTag = "cancelTaskBytag";
const char *kCancelAllTasks = "cancelAllTasks";
const char *kUnknown = "unknown";

const char *kOnResultSendMethod = "onResultSend";
const char *kBackgroundChannelInitialized = "backgroundChannelInitialized";

}  // namespace methods

namespace keys {
const char *kIsInDebugModeKey = "isInDebugMode";
const char *kCallhandlekey = "callbackHandle";

const char *kUniquenameKey = "uniqueName";
const char *kNameValueKey = "taskName";
const char *kTagKey = "tag";
const char *kExistingWorkpolicykey = "existingWorkPolicy";

const char *kNetworkTypekey = "networkType";
const char *kBatteryNotLowKey = "requiresBatteryNotLow";
const char *kChargingKey = "requiresCharging";
const char *kDeviceidlekey = "requiresDeviceIdle";
const char *kStorageNotLowKey = "requiresStorageNotLow";

const char *kInitialDelaySecondsKey = "initialDelaySeconds";
const char *kBackOffPolicyTypeKey = "backoffPolicyType";
const char *kBackOffPolicyDelayMillisKey = "backoffDelayInMilliseconds";
const char *kOutofQuotaPolicyKey = "outOfQuotaPolicy";
const char *kPayloadKey = "inputData";

const char *kFrequencySecondsKey = "frequency";

const char *kCancelTaskTagKey = "tag";
const char *kCancelTaskUniqueNameKey = "uniqueName";

const char *kBgChannelInputDataKey = "be.tramckrijte.workmanager.INPUT_DATA";
const char *kBgChannelDartTaskKey = "be.tramckrijte.workmanager.DART_TASK";

const char *kDispatcherHandleKey = "WMANAGER_TIZEN_DISPATCHER_HANDLE_KEY";

}  // namespace keys

namespace values {

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

}  // namespace values

const char *kForegroundChannelName =
    "be.tramckrijte.workmanager/foreground_channel_work_manager";
const char *kBackgroundChannelName =
    "be.tramckrijte.workmanager/background_channel_work_manager";

const char *kNotInitializedErrMsg =
    "You have not properly initialized the Flutter WorkManager Package. "
    "You should ensure you have called the 'initialize' function first! "
    "Example: \n"
    "\n"
    "`Workmanager().initialize(\n"
    "  callbackDispatcher,\n"
    " )`"
    "\n"
    "\n"
    "The `callbackDispatcher` is a top level function. See example in "
    "repository.";

}  // namespace constants

#endif
