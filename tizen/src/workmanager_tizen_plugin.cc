#include "workmanager_tizen_plugin.h"

#include <app.h>
#include <app_event.h>
#include <app_manager.h>
#include <app_preference.h>
#include <bundle.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <job_scheduler.h>
#include <system_info.h>
#include <tizen.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "extractor.h"
#include "job_scheduler_wrapper.h"
#include "log.h"

namespace {

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

const char *kForegroundChannelName =
    "be.tramckrijte.workmanager/foreground_channel_work_manager";
const char *kBackgroundChannelName =
    "be.tramckrijte.workmanager/background_channel_work_manager";

const char *kMethodNameKey = "methodName";
const char *kInitialize = "initialize";
const char *kRegisterOneOffTask = "registerOneOffTask";
const char *kRegisterPeriodicTask = "registerPeriodicTask";
const char *kCancelTaskByUniqueName = "cancelTaskByUniqueName";
const char *kCancelTaskByTag = "cancelTaskBytag";
const char *kCancelAllTasks = "cancelAllTasks";
const char *kUnknown = "unknown";

const char *kOnResultSendMethod = "onResultSend";
const char *kBackgroundChannelInitialized = "backgroundChannelInitialized";

const char *kIsInDebugMode = "isInDebugMode";
const char *kCallbackhandle = "callbackHandle";
const char *kFrequencySeconds = "frequency";
const char *kCancelTaskTag = "tag";
const char *kCancelTaskUniqueName = "uniqueName";

const char *kUniquename = "uniqueName";
const char *kNameValue = "taskName";
const char *kTag = "tag";

const char *kBgChannelInputData = "be.tramckrijte.workmanager.INPUT_DATA";
const char *kBgChannelDartTask = "be.tramckrijte.workmanager.DART_TASK";
const char *kDispatcherHandle = "WMANAGER_TIZEN_DISPATCHER_HANDLE_KEY";

const char *kPayload = "inputData";

const char *kIsPeriodic = "isPeriodic";

const char *kConstraintsBundle = "constraintsBundle";
const char *kBackOffPolicyBundle = "backoffPolicyBundle";

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

const int32_t kMinBackOffPeriodic = 15 * 60 * 1000;
const int32_t kMinBackOffOneOff = 10 * 1000;

const char *kEventName = "pass_taskinfo_event";

const char *kInvalidArg = "Invalid argument";
const char *kOperationFailed = "Operation failed/cancelled";

void SendTerminateRequestBgApp(const char *service_id) {
    app_context_h context;
    int ret = app_manager_get_app_context(service_id, &context);
    if (ret != APP_MANAGER_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }

    ret = app_manager_request_terminate_bg_app(context);
    if (ret != APP_MANAGER_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }

    ret = app_context_destroy(context);
    if (ret != APP_MANAGER_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }
}

bool CheckAppIsRunning(const char *app_id) {
    app_context_h context;
    int ret = app_manager_get_app_context(app_id, &context);
    if (ret == APP_MANAGER_ERROR_NO_SUCH_APP) {
        return false;
    }

    if (ret != APP_MANAGER_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
        return false;
    }

    app_state_e state;
    app_context_get_app_state(context, &state);

    switch (state) {
        case APP_STATE_FOREGROUND:
        case APP_STATE_BACKGROUND:
        case APP_STATE_SERVICE:
            return true;
    }
    return false;
}

void SendLaunchRequest(const char *app_id) {
    app_control_h control;
    int ret = app_control_create(&control);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }

    ret = app_control_set_app_id(control, app_id);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }

    ret = app_control_send_launch_request(control, NULL, NULL);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }

    ret = app_control_destroy(control);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
    }
}

bool CheckIsServiceApp() {
    char *app_id;
    int ret = app_manager_get_app_id(getpid(), &app_id);
    if (ret != APP_MANAGER_ERROR_NONE) {
        LOG_ERROR("Failed to get app id: %s", get_error_message(ret));
        return false;
    }
    app_info_h app_info;
    app_info_app_component_type_e app_type;
    app_info_create(app_id, &app_info);
    app_info_get_app_component_type(app_info, &app_type);
    return app_type == APP_INFO_APP_COMPONENT_TYPE_SERVICE_APP;
}

class WorkmanagerTizenPlugin : public flutter::Plugin {
   public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
        auto plugin = std::make_unique<WorkmanagerTizenPlugin>();

        is_service_app_ = CheckIsServiceApp();

        if (is_service_app_) {
            background_channel_ = std::make_unique<FlMethodChannel>(
                registrar->messenger(), kBackgroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

            background_channel_.value()->SetMethodCallHandler(
                [plugin_pointer = plugin.get()](const auto &call, auto result) {
                    plugin_pointer->HandleBackground(call, std::move(result));
                });

            event_handler_h handler;
            std::string service_app_id = GetAppId().value();
            std::string event_id =
                "event." + service_app_id.substr(0, service_app_id.size() - 8) +
                "." + kEventName;
            event_add_event_handler(event_id.c_str(), TaskInfoCallback, nullptr,
                                    &handler);

            auto &scheduler = JobScheduler::instance();
            auto job_names = scheduler.GetAllJobs();
            job_service_callback_s callback = {StartJobCallback,
                                               StopJobCallback};
            for (const auto &name : job_names) {
                scheduler.SetCallback(name.c_str(), &callback);
            }

        } else {
            auto foreground_channel = std::make_unique<FlMethodChannel>(
                registrar->messenger(), kForegroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

            foreground_channel->SetMethodCallHandler(
                [plugin_pointer = plugin.get()](const auto &call, auto result) {
                    plugin_pointer->HandleWorkmanagerCall(call,
                                                          std::move(result));
                });
        }

        registrar->AddPlugin(std::move(plugin));
    }

    WorkmanagerTizenPlugin() {}

    virtual ~WorkmanagerTizenPlugin() {}

   private:
    static std::optional<std::unique_ptr<FlMethodChannel>> background_channel_;
    static bool is_service_app_;
    static job_service_callback_s callback_;

    void HandleWorkmanagerCall(const FlMethodCall &call,
                               std::unique_ptr<FlMethodResult> result) {
        const auto method_name = call.method_name();
        const auto &arguments = *call.arguments();

        std::string app_id = GetAppId().value();
        std::string event_id = "event." + app_id + "." + kEventName;

        const std::string service_app_id = GetAppId().value() + "_service";

        if (call.method_name() == kCancelAllTasks) {
            bundle *bund = bundle_create();
            if (!bund) {
                LOG_ERROR("Failed create bundle");
                result->Error(kOperationFailed, "Failed Creating bundle.");
            }

            bundle_add_str(bund, kMethodNameKey, method_name.c_str());
            int ret = event_publish_app_event(event_id.c_str(), bund);
            bundle_free(bund);

            if (ret != EVENT_ERROR_NONE) {
                LOG_ERROR("Failed publich app event: %s",
                          get_error_message(ret));

                result->Error(kOperationFailed, "Failed publish app event");
                return;
            }

            result->Success();
            return;
        }

        // TODO : Unify error codes with a constant
        if (!std::holds_alternative<flutter::EncodableMap>(arguments)) {
            result->Error(kInvalidArg, "No argument provided");
            return;
        }

        flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);

        if (method_name == kInitialize) {
            bool isDebugMode =
                std::get<bool>(map[flutter::EncodableValue(kIsInDebugMode)]);
            int64_t handle = std::get<int64_t>(
                map[flutter::EncodableValue(kCallbackhandle)]);

            preference_set_int(kDispatcherHandle, handle);

            if (!CheckAppIsRunning(service_app_id.c_str())) {
                SendLaunchRequest(service_app_id.c_str());
            }

            result->Success();
        } else if (method_name == kRegisterOneOffTask ||
                   method_name == kRegisterPeriodicTask) {
            const bool is_periodic = method_name == kRegisterPeriodicTask;

            bool is_debug_mode =
                std::get<bool>(map[flutter::EncodableValue(kIsInDebugMode)]);
            std::string unique_name = std::get<std::string>(
                map[flutter::EncodableValue(kUniquename)]);
            std::string task_name =
                std::get<std::string>(map[flutter::EncodableValue(kNameValue)]);
            std::string tag =
                GetOrNullFromEncodableMap<std::string>(&map, kTag).value_or("");
            ExistingWorkPolicy existing_work_policy =
                ExtractExistingWorkPolicyFromMap(map);
            int32_t initial_delay_seconds =
                GetOrNullFromEncodableMap<int32_t>(&map, kInitialDelaySeconds)
                    .value_or(0);
            int32_t frequency_seconds =
                GetOrNullFromEncodableMap<int32_t>(&map, kFrequencySeconds)
                    .value_or(0);

            Constraints constraints_config =
                ExtractConstraintConfigFromMap(map);
            OutOfQuotaPolicy out_of_quota_policy =
                ExtractOutOfQuotaPolicyFromMap(map);
            BackoffPolicyTaskConfig backoff_policy_config =
                ExtractBackoffPolicyConfigFromMap(
                    map, is_periodic ? kMinBackOffPeriodic : kMinBackOffOneOff);
            std::string payload =
                GetOrNullFromEncodableMap<std::string>(&map, kPayload)
                    .value_or("");

            bundle *bund = bundle_create();

            if (!bund) {
                LOG_ERROR("Failed create bundle");
                result->Error(kOperationFailed, "Failed Creating bundle.");
            }

            bundle_add_str(bund, kMethodNameKey, method_name.c_str());

            bundle_add_byte(bund, kIsInDebugMode, &is_debug_mode, sizeof(bool));
            bundle_add_str(bund, kUniquename, unique_name.c_str());
            bundle_add_str(bund, kNameValue, task_name.c_str());
            bundle_add_str(bund, kTag, tag.c_str());
            bundle_add_byte(bund, kExistingWorkpolicy, &existing_work_policy,
                            sizeof(ExistingWorkPolicy));

            bundle_add_byte(bund, kInitialDelaySeconds, &initial_delay_seconds,
                            sizeof(int32_t));
            bundle_add_byte(bund, kFrequencySeconds, &frequency_seconds,
                            sizeof(int32_t));

            bundle_add_str(bund, kPayload, payload.c_str());

            bundle_add_byte(bund, kConstraintsBundle, &constraints_config,
                            sizeof(Constraints));
            bundle_add_byte(bund, kBackOffPolicyBundle, &backoff_policy_config,
                            sizeof(BackoffPolicyTaskConfig));
            bundle_add_byte(bund, kOutofQuotaPolicy, &out_of_quota_policy,
                            sizeof(OutOfQuotaPolicy));
            bundle_add_byte(bund, kIsPeriodic, &is_periodic, sizeof(bool));

            int ret = event_publish_app_event(event_id.c_str(), bund);
            if (ret != EVENT_ERROR_NONE) {
                LOG_ERROR("Failed publish event: %s", get_error_message(ret));
                result->Error(kOperationFailed, "Error occured.");
                return;
            }
            bundle_free(bund);

            result->Success();
        } else if (method_name == kCancelTaskByUniqueName) {
            auto name = GetOrNullFromEncodableMap<std::string>(
                &map, kCancelTaskUniqueName);
            if (!name.has_value()) {
                result->Error(kInvalidArg, "No name provided");
                return;
            }

            bundle *bund = bundle_create();
            if (!bund) {
                LOG_ERROR("Failed create bundle");
                result->Error(kOperationFailed, "Failed Creating bundle.");
            }

            bundle_add_str(bund, kMethodNameKey, method_name.c_str());
            bundle_add_str(bund, kCancelTaskByUniqueName, name.value().c_str());

            int ret = event_publish_app_event(event_id.c_str(), bund);
            bundle_free(bund);

            if (ret != BUNDLE_ERROR_NONE) {
                LOG_ERROR("Failed publish event: %s", get_error_message(ret));
                result->Error(kOperationFailed, "Failed Publish event.");
                return;
            }

            result->Success();

        } else if (method_name == kCancelTaskByTag) {
            auto tag =
                GetOrNullFromEncodableMap<std::string>(&map, kCancelTaskTag);
            if (!tag.has_value()) {
                result->Error(kInvalidArg, "No tag provided");
                return;
            }

            bundle *bund = bundle_create();
            if (!bund) {
                LOG_ERROR("Failed create bundle");
                result->Error(kOperationFailed, "Failed Creating bundle.");
            }

            bundle_add_str(bund, kMethodNameKey, method_name.c_str());
            bundle_add_str(bund, kCancelTaskTag, tag.value().c_str());

            int ret = event_publish_app_event(event_id.c_str(), bund);
            if (ret != EVENT_ERROR_NONE) {
                LOG_ERROR("Failed publish event: %s", get_error_message(ret));
                result->Error(kOperationFailed, "Failed publish event.");
                bundle_free(bund);
                return;
            }

            bundle_free(bund);

            result->Success();

        } else {
            result->NotImplemented();
        }
    }

    void HandleBackground(const FlMethodCall &call,
                          std::unique_ptr<FlMethodResult> result) {
        if (call.method_name() == kBackgroundChannelInitialized) {
            // check initialized
        }

        result->Success();
    }

    static void RunBackgroundCallback(const std::string &job_id,
                                      const std::string &payload) {
        if (!background_channel_.has_value()) {
            LOG_ERROR("Background channel is not initialized.");
            return;
        }

        flutter::EncodableMap arg = {
            {flutter::EncodableValue(kBgChannelDartTask),
             flutter::EncodableValue(job_id)},
        };

        if (payload.size() > 0) {
            arg[flutter::EncodableValue(kBgChannelInputData)] =
                flutter::EncodableValue(payload);
        }

        background_channel_.value()->InvokeMethod(
            kOnResultSendMethod,
            std::make_unique<flutter::EncodableValue>(arg));
    }

    static void StartJobCallback(job_info_h job_info, void *user_data) {
        char *job_id = nullptr;
        job_info_get_job_id(job_info, &job_id);

        std::string job_id_str(job_id);
        std::string preference_key(kPayloadPreferencePrefix + job_id_str);

        char *payload;
        preference_get_string(preference_key.c_str(), &payload);

        RunBackgroundCallback(job_id_str, payload);
    }

    static void StopJobCallback(job_info_h job_info, void *user_data) {
        // Empty
    }

    static std::optional<std::string> GetAppId() {
        char *app_id;
        int ret = app_manager_get_app_id(getpid(), &app_id);
        if (ret == 0) {
            return std::string(app_id);
        }

        return std::nullopt;
    }

    static void TaskInfoCallback(const char *event_name, bundle *event_data,
                                 void *user_data) {
        size_t size;
        char *method_name = nullptr;

        bundle_get_str(event_data, kMethodNameKey, &method_name);

        std::string method_name_str(method_name);

        auto &job_scheduler = JobScheduler::instance();

        if (method_name_str == kRegisterOneOffTask ||
            method_name_str == kRegisterPeriodicTask) {
            bool *is_debug_mode = nullptr;
            char *unique_name = nullptr;
            char *task_name = nullptr;
            char *tag = nullptr;

            ExistingWorkPolicy *existing_work_policy = nullptr;

            int32_t *initial_delay_seconds = nullptr;
            int32_t *frequency_seconds = nullptr;
            char *payload = nullptr;
            bool *is_periodic = nullptr;

            Constraints *constraints = nullptr;
            BackoffPolicyTaskConfig *backoff_policy = nullptr;
            OutOfQuotaPolicy *out_of_quota_policy = nullptr;

            bundle_get_byte(event_data, kIsInDebugMode, (void **)&is_debug_mode,
                            &size);
            bundle_get_str(event_data, kUniquename, &unique_name);
            bundle_get_str(event_data, kNameValue, &task_name);
            bundle_get_str(event_data, kTag, &tag);
            bundle_get_byte(event_data, kExistingWorkpolicy,
                            (void **)&existing_work_policy, &size);

            bundle_get_byte(event_data, kInitialDelaySeconds,
                            (void **)&initial_delay_seconds, &size);
            bundle_get_byte(event_data, kFrequencySeconds,
                            (void **)&frequency_seconds, &size);
            bundle_get_str(event_data, kPayload, &payload);
            bundle_get_byte(event_data, kIsPeriodic, (void **)&is_periodic,
                            &size);

            bundle_get_byte(event_data, kConstraintsBundle,
                            (void **)&constraints, &size);
            bundle_get_byte(event_data, kBackOffPolicyBundle,
                            (void **)&backoff_policy, &size);
            bundle_get_byte(event_data, kOutofQuotaPolicy,
                            (void **)&out_of_quota_policy, &size);
            bundle_get_byte(event_data, kIsPeriodic, (void **)&is_periodic,
                            &size);

            if (*is_periodic) {
                job_service_callback_s callback = {StartJobCallback,
                                                   StopJobCallback};

                job_scheduler.RegisterJob(
                    *is_debug_mode, unique_name, task_name,
                    *existing_work_policy, *initial_delay_seconds, *constraints,
                    *backoff_policy, *out_of_quota_policy, *is_periodic,
                    *frequency_seconds / 60, tag, payload, &callback);

            } else {
                RunBackgroundCallback(unique_name, payload);
            }

        } else if (method_name_str == kCancelTaskByUniqueName) {
            char *unique_name;
            bundle_get_str(event_data, kUniquename, &unique_name);

            job_scheduler.CancelByUniqueName(unique_name);

        } else if (method_name_str == kCancelAllTasks) {
            job_scheduler.CancelAll();
        }
    }
};

std::optional<std::unique_ptr<FlMethodChannel>>
    WorkmanagerTizenPlugin::background_channel_ = std::nullopt;
bool WorkmanagerTizenPlugin::is_service_app_ = false;

}  // namespace

void WorkmanagerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
    WorkmanagerTizenPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()
            ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
