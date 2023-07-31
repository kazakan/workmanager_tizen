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

const char *kIsInDebugModeKey = "isInDebugMode";
const char *kCallhandlekey = "callbackHandle";
const char *kFrequencySecondsKey = "frequency";
const char *kCancelTaskTagKey = "tag";
const char *kCancelTaskUniqueNameKey = "uniqueName";

const char *kUniquenameKey = "uniqueName";
const char *kNameValueKey = "taskName";
const char *kTagKey = "tag";

const char *kBgChannelInputDataKey = "be.tramckrijte.workmanager.INPUT_DATA";
const char *kBgChannelDartTaskKey = "be.tramckrijte.workmanager.DART_TASK";
const char *kDispatcherHandleKey = "WMANAGER_TIZEN_DISPATCHER_HANDLE_KEY";

const char *kPayloadKey = "inputData";

const char *kIsPeriodicKey = "isPeriodic";

const char *kConstraintsBundleKey = "constraintsBundle";
const char *kBackOffPolicyBundleKey = "backoffPolicyBundle";

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

class WorkmanagerTizenPlugin : public flutter::Plugin {
   public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
        auto plugin = std::make_unique<WorkmanagerTizenPlugin>();

        auto foreground_channel = std::make_unique<FlMethodChannel>(
            registrar->messenger(), kForegroundChannelName,
            &flutter::StandardMethodCodec::GetInstance());

        foreground_channel->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto &call, auto result) {
                plugin_pointer->HandleWorkmanagerCall(call, std::move(result));
            });

        is_service_app_ = CheckIsServiceApp();

        if (is_service_app_) {
            background_channel_ = std::make_unique<FlMethodChannel>(
                registrar->messenger(), kBackgroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

            background_channel_.value()->SetMethodCallHandler(
                [plugin_pointer = plugin.get()](const auto &call, auto result) {
                    plugin_pointer->HandleBackground(call, std::move(result));
                });

            auto &scheduler = JobScheduler::instance();
            job_service_callback_s callback = {StartJobCallback,
                                               StopJobCallback};
            auto job_names = scheduler.GetAllJobs();

            event_handler_h handler;
            std::string service_app_id = GetAppId().value();
            std::string event_id =
                "event." + service_app_id.substr(0, service_app_id.size() - 8) +
                "." + kEventName;
            int err = event_add_event_handler(
                event_id.c_str(), TaskInfoCallback, nullptr, &handler);
        }

        registrar->AddPlugin(std::move(plugin));
    }

    WorkmanagerTizenPlugin() {}

    virtual ~WorkmanagerTizenPlugin() {}

   private:
    static std::optional<std::unique_ptr<FlMethodChannel>> background_channel_;
    static bool is_service_app_;

    void HandleWorkmanagerCall(const FlMethodCall &call,
                               std::unique_ptr<FlMethodResult> result) {
        const auto method_name = call.method_name();
        const auto &arguments = *call.arguments();
        auto &job_scheduler = JobScheduler::instance();

        SendLaunchRequest((GetAppId().value() + "_service").c_str());

        if (call.method_name() == kCancelAllTasks) {
            job_scheduler.CancelAll();

            result->Success();
            return;
        }

        // TODO : Unify error codes with a constant
        if (!std::holds_alternative<flutter::EncodableMap>(arguments)) {
            result->Error("WRONG_ARGS", "No argument provided");
            return;
        }

        flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);
        std::string app_id = GetAppId().value();
        std::string event_id = "event." + app_id + "." + kEventName;

        if (method_name == kInitialize) {
            bool isDebugMode =
                std::get<bool>(map[flutter::EncodableValue(kIsInDebugModeKey)]);
            int64_t handle =
                std::get<int64_t>(map[flutter::EncodableValue(kCallhandlekey)]);
            const auto &info = InitializeTaskInfo(handle, isDebugMode);

            preference_set_int(kDispatcherHandleKey,
                               info.callback_dispathcer_handler_key);
            result->Success();
        } else if (method_name == kRegisterOneOffTask ||
                   method_name == kRegisterPeriodicTask) {
            const bool is_periodic = method_name == kRegisterPeriodicTask;

            bool is_debug_mode =
                std::get<bool>(map[flutter::EncodableValue(kIsInDebugModeKey)]);
            std::string unique_name = std::get<std::string>(
                map[flutter::EncodableValue(kUniquenameKey)]);
            std::string task_name = std::get<std::string>(
                map[flutter::EncodableValue(kNameValueKey)]);
            std::string tag =
                GetOrNullFromEncodableMap<std::string>(&map, kTagKey)
                    .value_or("");
            ExistingWorkPolicy existing_work_policy =
                ExtractExistingWorkPolicyFromMap(map);
            int32_t initial_delay_seconds = GetOrNullFromEncodableMap<int32_t>(
                                                &map, kInitialDelaySecondsKey)
                                                .value_or(0);
            int32_t frequency_seconds =
                GetOrNullFromEncodableMap<int32_t>(&map, kFrequencySecondsKey)
                    .value_or(0);

            Constraints constraints_config =
                ExtractConstraintConfigFromMap(map);
            OutOfQuotaPolicy out_of_quota_policy =
                ExtractOutOfQuotaPolicyFromMap(map);
            BackoffPolicyTaskConfig backoff_policy_config =
                ExtractBackoffPolicyConfigFromMap(
                    map, is_periodic ? kMinBackOffPeriodic : kMinBackOffOneOff);
            std::string payload =
                GetOrNullFromEncodableMap<std::string>(&map, kPayloadKey)
                    .value_or("");

            const auto &info = RegisterTaskInfo(
                is_debug_mode, unique_name, task_name, existing_work_policy,
                initial_delay_seconds, constraints_config,
                backoff_policy_config, out_of_quota_policy, frequency_seconds,
                tag, payload, is_periodic);

            bundle *bund = nullptr;
            bund = bundle_create();

            bundle_add_str(bund, kMethodNameKey, method_name.c_str());

            bundle_add_byte(bund, kIsInDebugModeKey, &is_debug_mode, 1);
            bundle_add_str(bund, kUniquenameKey, unique_name.c_str());
            bundle_add_str(bund, kNameValueKey, task_name.c_str());
            bundle_add_str(bund, kTagKey, tag.c_str());
            bundle_add_byte(bund, kExistingWorkpolicykey, &existing_work_policy,
                            sizeof(ExistingWorkPolicy));

            bundle_add_byte(bund, kInitialDelaySecondsKey,
                            &initial_delay_seconds, 4);
            bundle_add_byte(bund, kFrequencySecondsKey, &frequency_seconds, 4);

            bundle_add_str(bund, kPayloadKey, payload.c_str());

            bundle_add_byte(bund, kConstraintsBundleKey, &constraints_config,
                            sizeof(Constraints));
            bundle_add_byte(bund, kBackOffPolicyBundleKey,
                            &backoff_policy_config,
                            sizeof(BackoffPolicyTaskConfig));
            bundle_add_byte(bund, kOutofQuotaPolicyKey, &out_of_quota_policy,
                            sizeof(OutOfQuotaPolicy));

            LOG_DEBUG("event_id: %s", event_id.c_str());
            int err = event_publish_app_event(event_id.c_str(), bund);
            if (err) {
                LOG_ERROR("Failed publish event: %s", get_error_message(err));
                result->Error("Error publish event", "Error occured.");
                bundle_free(bund);
                return;
            }

            bundle_free(bund);

            result->Success();
        } else if (method_name == kCancelTaskByUniqueName) {
            auto name = GetOrNullFromEncodableMap<std::string>(
                &map, kCancelTaskUniqueNameKey);
            if (!name.has_value()) {
                result->Error("WRONG ARGS", "No name provided");
                return;
            }

            bundle *bund = nullptr;
            bund = bundle_create();
            bundle_add_str(bund, kMethodNameKey, method_name.c_str());
            bundle_add_str(bund, kCancelTaskByUniqueName, name.value().c_str());

            int err = event_publish_app_event(event_id.c_str(), bund);
            if (err) {
                LOG_ERROR("Failed publish event: %s", get_error_message(err));
                result->Error("Error publish event", "Error occured.");
                bundle_free(bund);
                return;
            }

            bundle_free(bund);

            result->Success();

        } else if (method_name == kCancelTaskByTag) {
            auto tag =
                GetOrNullFromEncodableMap<std::string>(&map, kCancelTaskTagKey);
            if (!tag.has_value()) {
                result->Error("WRONG ARGS", "No name provided");
                return;
            }

            bundle *bund = nullptr;
            bund = bundle_create();
            bundle_add_str(bund, kMethodNameKey, method_name.c_str());
            bundle_add_str(bund, kCancelTaskTagKey, tag.value().c_str());

            int err = event_publish_app_event(event_id.c_str(), bund);
            if (err) {
                LOG_ERROR("Failed publish event: %s", get_error_message(err));
                result->Error("Error publish event", "Error occured.");
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
            const auto &args = *call.arguments();
            if (!std::holds_alternative<flutter::EncodableMap>(args)) {
                result->Error("WRONG ARGS",
                              "No proper argument provided for background");
                return;
            }
            flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);
            auto dart_task =
                map[flutter::EncodableValue(kBgChannelDartTaskKey)];
            auto input_data =
                map[flutter::EncodableValue(kBgChannelInputDataKey)];

            flutter::EncodableMap arg = {
                {flutter::EncodableValue(std::string(kBgChannelDartTaskKey)),
                 dart_task},
                {flutter::EncodableValue(std::string(kBgChannelInputDataKey)),
                 input_data}};

            background_channel_.value()->InvokeMethod(
                kOnResultSendMethod,
                std::make_unique<flutter::EncodableValue>(arg));
        }
    }

    static bool CheckIsServiceApp() {
        char *app_id;
        int err = app_manager_get_app_id(getpid(), &app_id);
        if (err) {
            LOG_ERROR("Failed to get app id: %s", get_error_message(err));
            return false;
        }
        app_info_h app_info;
        app_info_app_component_type_e app_type;
        app_info_create(app_id, &app_info);
        app_info_get_app_component_type(app_info, &app_type);
        return app_type == APP_INFO_APP_COMPONENT_TYPE_SERVICE_APP;
    }

    static void RunBackgroundCallback(const std::string job_id,
                                      const std::string payload) {
        flutter::EncodableMap arg = {
            {flutter::EncodableValue(kBgChannelInputDataKey),
             flutter::EncodableValue(payload)},
            {flutter::EncodableValue(kBgChannelDartTaskKey),
             flutter::EncodableValue(job_id)}};

        background_channel_.value()->InvokeMethod(
            kOnResultSendMethod,
            std::make_unique<flutter::EncodableValue>(arg));
    }

    static void StartJobCallback(job_info_h job_info, void *user_data) {
        if (!background_channel_.has_value()) {
            return;
        }

        char *job_id = nullptr;
        job_info_get_job_id(job_info, &job_id);

        std::string payload(static_cast<char *>(user_data));
        RunBackgroundCallback(std::string(job_id), payload);
    }

    static void StopJobCallback(job_info_h job_info, void *user_data) {
        // Currently do nothing.
    }

    static std::optional<std::string> GetAppId() {
        char *app_id;
        int err = app_manager_get_app_id(getpid(), &app_id);
        if (err == 0) {
            return std::string(app_id);
        }

        return std::nullopt;
    }

    static void TaskInfoCallback(const char *event_name, bundle *event_data,
                                 void *user_data) {
        LOG_DEBUG("event_name is [%s]", event_name);

        size_t size;
        char *method_name = nullptr;

        bundle *bund = bundle_create();
        bundle_get_str(bund, kMethodNameKey, &method_name);

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

            bundle_get_byte(bund, kIsInDebugModeKey, (void **)&is_debug_mode,
                            &size);
            bundle_get_str(bund, kUniquenameKey, &unique_name);
            bundle_get_str(bund, kNameValueKey, &task_name);
            bundle_get_byte(bund, kExistingWorkpolicykey,
                            (void **)&existing_work_policy, &size);

            bundle_get_byte(bund, kInitialDelaySecondsKey,
                            (void **)&initial_delay_seconds, &size);
            bundle_get_byte(bund, kFrequencySecondsKey,
                            (void **)&frequency_seconds, &size);
            bundle_get_str(bund, kPayloadKey, &payload);
            bundle_get_byte(bund, kIsPeriodicKey, (void **)&is_periodic, &size);

            bundle_get_byte(bund, kConstraintsBundleKey, (void **)&constraints,
                            &size);
            bundle_get_byte(bund, kBackOffPolicyBundleKey,
                            (void **)&backoff_policy, &size);
            bundle_get_byte(bund, kOutofQuotaPolicyKey,
                            (void **)&out_of_quota_policy, &size);
            bundle_get_byte(bund, kIsPeriodicKey, (void **)&is_periodic, &size);

            if (*is_periodic) {
                job_scheduler.RegisterJob(
                    is_debug_mode, unique_name, task_name,
                    *existing_work_policy, *initial_delay_seconds, *constraints,
                    *backoff_policy, *out_of_quota_policy, *is_periodic,
                    *frequency_seconds, tag, payload);
            } else {
                RunBackgroundCallback(unique_name,payload);
            }

        } else if (method_name_str == kCancelTaskByUniqueName) {
            char *unique_name;
            bundle_get_str(bund, kUniquenameKey, &unique_name);

            job_scheduler.CancelByUniqueName(unique_name);

        } else if (method_name_str == kCancelAllTasks) {
            job_scheduler.CancelAll();
        }

        bundle_free(bund);
    }

    void SendLaunchRequest(const char *app_id) {
        app_control_h control;
        int ret = app_control_create(&control);
        if (ret) {
            LOG_ERROR("%s", get_error_message(ret));
        }

        ret = app_control_set_app_id(control, app_id);
        if (ret) {
            LOG_ERROR("%s", get_error_message(ret));
        }

        ret = app_control_send_launch_request(control, NULL, NULL);
        if (ret) {
            LOG_ERROR("%s", get_error_message(ret));
        }

        ret = app_control_destroy(control);
        if (ret) {
            LOG_ERROR("%s", get_error_message(ret));
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
