#include "workmanager_tizen_plugin.h"

#include <app_manager.h>
#include <app_preference.h>
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

            LOG_DEBUG("Show current jobs");
            for (const auto &name : job_names) {
                LOG_DEBUG("%s",name.c_str());
                scheduler.SetCallback(name, callback, nullptr);
            }

            LOG_DEBUG("Show current jobs - end");
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
            const bool isPeriodic = method_name == kRegisterPeriodicTask;

            bool is_debug_mode =
                std::get<bool>(map[flutter::EncodableValue(kIsInDebugModeKey)]);
            std::string unique_name = std::get<std::string>(
                map[flutter::EncodableValue(kUniquenameKey)]);
            std::string task_name = std::get<std::string>(
                map[flutter::EncodableValue(kNameValueKey)]);
            std::optional<std::string> tag =
                GetOrNullFromEncodableMap<std::string>(&map, kTagKey);
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
            std::optional<OutOfQuotaPolicy> out_of_quota_policy =
                ExtractOutOfQuotaPolicyFromMap(map);
            std::optional<BackoffPolicyTaskConfig> backoff_policy_config =
                ExtractBackoffPolicyConfigFromMap(
                    map, isPeriodic ? kMinBackOffPeriodic : kMinBackOffOneOff);
            std::optional<std::string> payload =
                GetOrNullFromEncodableMap<std::string>(&map, kPayloadKey);

            const auto &info = RegisterTaskInfo(
                is_debug_mode, unique_name, task_name, existing_work_policy,
                initial_delay_seconds, constraints_config,
                backoff_policy_config, out_of_quota_policy, frequency_seconds,
                tag, payload);

            job_scheduler.RegisterJob(info, isPeriodic);

            result->Success();
        } else if (method_name == kCancelTaskByUniqueName) {
            auto name = GetOrNullFromEncodableMap<std::string>(
                &map, kCancelTaskUniqueNameKey);
            if (!name.has_value()) {
                result->Error("WRONG ARGS", "No name provided");
                return;
            }

            job_scheduler.CancelByUniqueName(name.value());

            result->Success();

        } else if (method_name == kCancelTaskByTag) {
            auto tag =
                GetOrNullFromEncodableMap<std::string>(&map, kCancelTaskTagKey);
            if (!tag.has_value()) {
                result->Error("WRONG ARGS", "No name provided");
                return;
            }

            job_scheduler.CancelByTag(tag.value());

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

    static void StartJobCallback(job_info_h job_info, void *user_data) {
        if (!background_channel_.has_value()) {
            return;
        }

        char *job_id = nullptr;
        job_info_get_job_id(job_info, &job_id);

        std::string payload(static_cast<char *>(user_data));

        flutter::EncodableMap arg = {
            {flutter::EncodableValue(kBgChannelInputDataKey),
             flutter::EncodableValue(payload)},
            {flutter::EncodableValue(kBgChannelDartTaskKey),
             flutter::EncodableValue(std::string(job_id))}};

        background_channel_.value()->InvokeMethod(
            kOnResultSendMethod,
            std::make_unique<flutter::EncodableValue>(arg));
    }

    static void StopJobCallback(job_info_h job_info, void *user_data) {
        // Currently do nothing.
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
