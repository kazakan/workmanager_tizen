#include "workmanager_tizen_plugin.h"

#include <app_preference.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <job_scheduler.h>
#include <system_info.h>
#include <tizen.h>

#include <memory>
#include <string>

#include "extractor.h"
#include "job_scheduler_wrapper.h"
#include "log.h"

namespace {

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

class WorkmanagerTizenPlugin : public flutter::Plugin {
   public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
        auto plugin = std::make_unique<WorkmanagerTizenPlugin>();

        auto foreground_channel = std::make_unique<FlMethodChannel>(
            registrar->messenger(), constants::kForegroundChannelName,
            &flutter::StandardMethodCodec::GetInstance());

        plugin->background_channel_ = std::make_unique<FlMethodChannel>(
            registrar->messenger(), constants::kBackgroundChannelName,
            &flutter::StandardMethodCodec::GetInstance());

        foreground_channel->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                plugin_pointer->HandleWorkmanagerCall(call, std::move(result));
            });

        plugin->background_channel_->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                plugin_pointer->HandleBackground(call, std::move(result));
            });

        registrar->AddPlugin(std::move(plugin));
    }

    WorkmanagerTizenPlugin() {}

    virtual ~WorkmanagerTizenPlugin() {}

   private:
    std::unique_ptr<FlMethodChannel> background_channel_;

    void HandleWorkmanagerCall(const FlMethodCall& call,
                               std::unique_ptr<FlMethodResult> result) {
        const auto method_name = call.method_name();
        const auto& arguments = *call.arguments();
        JobScheduler job_scheduler;

        if (call.method_name() == constants::methods::kCancelAllTasks) {
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

        if (method_name == constants::methods::kInitialize) {
            bool isDebugMode = std::get<bool>(map[flutter::EncodableValue(
                constants::keys::kIsInDebugModeKey)]);
            int64_t handle = std::get<int64_t>(
                map[flutter::EncodableValue(constants::keys::kCallhandlekey)]);
            const auto& call = InitializeTask(handle, isDebugMode);

            preference_set_int(constants::keys::kDispatcherHandleKey,
                               call.callback_dispathcer_handler_key);
            result->Success();
        } else if (method_name == constants::methods::kRegisterOneOffTask ||
                   method_name == constants::methods::kRegisterPeriodicTask) {
            bool is_debug_mode = std::get<bool>(map[flutter::EncodableValue(
                constants::keys::kIsInDebugModeKey)]);
            std::string unique_name = std::get<std::string>(
                map[flutter::EncodableValue(constants::keys::kUniquenameKey)]);
            std::string task_name = std::get<std::string>(
                map[flutter::EncodableValue(constants::keys::kNameValueKey)]);
            std::optional<std::string> tag =
                GetOrNullFromEncodableMap<std::string>(
                    &map, constants::keys::kTagKey);
            ExistingWorkPolicy existing_work_policy =
                ExtractExistingWorkPolicyFromMap(map);
            int32_t initial_delay_seconds =
                GetOrNullFromEncodableMap<int32_t>(
                    &map, constants::keys::kInitialDelaySecondsKey)
                    .value_or(0);
            int32_t frequency_seconds =
                GetOrNullFromEncodableMap<int32_t>(
                    &map, constants::keys::kFrequencySecondsKey)
                    .value_or(0);

            Constraints constraints_config =
                ExtractConstraintConfigFromMap(map);
            std::optional<OutOfQuotaPolicy> out_of_quota_policy =
                ExtractOutOfQuotaPolicyFromMap(map);
            std::optional<BackoffPolicyTaskConfig> backoff_policy_config =
                ExtractBackoffPolicyConfigFromMap(map,
                                                  TaskType(TaskType::kOneOff));
            std::optional<std::string> payload =
                GetOrNullFromEncodableMap<std::string>(
                    &map, constants::keys::kPayloadKey);

            LOG_DEBUG("freq=%ld tag=%s payload=%s", frequency_seconds,
                      tag.value_or("no tag").c_str(),
                      payload.value_or("no payload").c_str());

            const auto& call = RegisterTask(
                is_debug_mode, unique_name, task_name, existing_work_policy,
                initial_delay_seconds, constraints_config,
                backoff_policy_config, out_of_quota_policy, frequency_seconds,
                tag, payload);
            const bool isPeriodic =
                method_name == constants::methods::kRegisterPeriodicTask;

            job_scheduler.RegisterJob(call, isPeriodic);

            result->Success();
        } else if (method_name == constants::methods::kCancelTaskByUniqueName) {
            auto name = GetOrNullFromEncodableMap<std::string>(
                &map, constants::keys::kCancelTaskUniqueNameKey);
            if (!name.has_value()) {
                result->Error("WRONG ARGS", "No name provided");
                return;
            }
            auto taskInfo = CancelTaskInfo();
            taskInfo.name = name.value();

            job_scheduler.CancelByUniqueName(taskInfo);

            result->Success();

        } else if (method_name == constants::methods::kCancelTaskByTag) {
            auto tag = GetOrNullFromEncodableMap<std::string>(
                &map, constants::keys::kCancelTaskTagKey);
            if (!tag.has_value()) {
                result->Error("WRONG ARGS", "No name provided");
                return;
            }
            auto taskInfo = CancelTaskInfo();
            taskInfo.tag = tag.value();

            job_scheduler.CancelByTag(taskInfo);

            result->Success();

        } else {
            result->NotImplemented();
        }
    }

    void HandleBackground(const FlMethodCall& call,
                          std::unique_ptr<FlMethodResult> result) {
        LOG_DEBUG("Background call name =%s", call.method_name().c_str());

        if (call.method_name() ==
            constants::methods::kBackgroundChannelInitialized) {
            const auto& args = *call.arguments();
            if (!std::holds_alternative<flutter::EncodableMap>(args)) {
                result->Error("WRONG ARGS",
                              "No proper argument provided for background");
            }
            flutter::EncodableMap map = std::get<flutter::EncodableMap>(args);
            auto dart_task = map[flutter::EncodableValue(
                constants::keys::kBgChannelDartTaskKey)];
            auto input_data = map[flutter::EncodableValue(
                constants::keys::kBgChannelInputDataKey)];

            flutter::EncodableMap arg = {
                {flutter::EncodableValue(
                     std::string(constants::keys::kBgChannelDartTaskKey)),
                 dart_task},
                {flutter::EncodableValue(
                     std::string(constants::keys::kBgChannelInputDataKey)),
                 input_data}};

            background_channel_->InvokeMethod(
                constants::methods::kOnResultSendMethod,
                std::make_unique<flutter::EncodableValue>(arg));
        }
    }
};

}  // namespace

void WorkmanagerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
    WorkmanagerTizenPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()
            ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
