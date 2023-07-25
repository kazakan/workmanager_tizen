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

        if (method_name == constants::methods::kInitialize) {
            HandleInitializeTask(arguments, std::move(result));
        } else if (method_name == constants::methods::kRegisterOneOffTask) {
            HandleOneOffTask(arguments, std::move(result));
        } else if (method_name == constants::methods::kRegisterPeriodicTask) {
            HandlePeriodicTask(arguments, std::move(result));
        } else if (method_name == constants::methods::kCancelTaskByUniqueName) {
            HandleCancelByName(arguments, std::move(result));
        } else if (method_name == constants::methods::kCancelTaskByTag) {
            HandleCancelByTag(arguments, std::move(result));
        } else if (call.method_name() == constants::methods::kCancelAllTasks) {
            HandleCancelAll(std::move(result));
        } else {
            result->NotImplemented();
        }
    }

    void HandleInitializeTask(const flutter::EncodableValue& arguments,
                              std::unique_ptr<FlMethodResult> result) {
        if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
            flutter::EncodableMap map =
                std::get<flutter::EncodableMap>(arguments);
            bool isDebugMode = std::get<bool>(map[flutter::EncodableValue(
                constants::keys::kIsInDebugModeKey)]);
            int64_t handle = std::get<int64_t>(
                map[flutter::EncodableValue(constants::keys::kCallhandlekey)]);
            const auto& call = InitializeTask(handle, isDebugMode);

            preference_set_int(constants::keys::kDispatcherHandleKey,
                               call.callback_dispathcer_handler_key);
            result->Success();
        } else {
            result->Error("WRONG_ARGS", "Wrong argument for Initialize");
        }
    }

    void HandleOneOffTask(const flutter::EncodableValue& arguments,
                          std::unique_ptr<FlMethodResult> result) {
        if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
            flutter::EncodableMap map =
                std::get<flutter::EncodableMap>(arguments);
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

            Constraints constraints_config =
                ExtractConstraintConfigFromCall(map);
            std::optional<OutOfQuotaPolicy> out_of_quota_policy =
                ExtractOutOfQuotaPolicyFromMap(map);
            std::optional<BackoffPolicyTaskConfig> backoff_policy_config =
                ExtractBackoffPolicyConfigFromMap(map,
                                                  TaskType(TaskType::kOneOff));
            std::optional<std::string> payload =
                GetOrNullFromEncodableMap<std::string>(
                    &map, constants::keys::kPayloadKey);

            const auto& call = OneoffTask(
                is_debug_mode, unique_name, task_name, existing_work_policy,
                initial_delay_seconds, constraints_config,
                backoff_policy_config, out_of_quota_policy, tag, payload);

            bool initialized = false;
            preference_is_existing(constants::keys::kDispatcherHandleKey,
                                   &initialized);
            if (!initialized) {
                result->Error("1", constants::kNotInitializedErrMsg, nullptr);
                return;
            }

            JobScheduler job_scheduler;
            job_scheduler.RegisterOneOffJob(call);

            result->Success();
        } else {
            result->Error("WRONG_ARGS",
                          "Wrong argument for registerOneOffTask");
        }
    }

    void HandlePeriodicTask(const flutter::EncodableValue& arguments,
                            std::unique_ptr<FlMethodResult> result) {
        if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
            flutter::EncodableMap map =
                std::get<flutter::EncodableMap>(arguments);

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
                ExtractConstraintConfigFromCall(map);
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

            const auto& call = PeriodicTask(
                is_debug_mode, unique_name, task_name, existing_work_policy,
                frequency_seconds, initial_delay_seconds, constraints_config,
                backoff_policy_config, out_of_quota_policy, tag, payload);

            JobScheduler job_scheduler;

            LOG_DEBUG("Periodictask name=%s freq=%d", call.unique_name.c_str(),
                      call.frequency_in_seconds);

            job_scheduler.RegisterPeriodicJob(call);

            result->Success();
        } else {
            result->Error("WRONG_ARGS",
                          "Wrong argument for registerPeriodicTask");
        }
    }

    void HandleCancelByTag(const flutter::EncodableValue& arguments,
                           std::unique_ptr<FlMethodResult> result) {
        if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
            flutter::EncodableMap map =
                std::get<flutter::EncodableMap>(arguments);
            auto value = GetOrNullFromEncodableMap<std::string>(
                &map, constants::keys::kCancelTaskTagKey);
            if (value.has_value()) {
                const auto& call = CancelByTagTask(value.value());

                JobScheduler job_scheduler;
                job_scheduler.CancelByTag(call);

                result->Success();
                return;
            }
        }
        result->Error("WRONG_ARGS", "Wrong argument for cancelTaskBytag");
    }

    void HandleCancelByName(const flutter::EncodableValue& arguments,
                            std::unique_ptr<FlMethodResult> result) {
        if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
            flutter::EncodableMap map =
                std::get<flutter::EncodableMap>(arguments);
            auto value = GetOrNullFromEncodableMap<std::string>(
                &map, constants::keys::kCancelTaskUniqueNameKey);
            if (value.has_value()) {
                const auto& call = CancelByNameTask(value.value());
                JobScheduler job_scheduler;
                job_scheduler.CancelByUniqueName(call);

                result->Success();
                return;
            }
        }
        result->Error("WRONG_ARGS",
                      "Wrong argument for cancelTaskByUniqueName");
    }

    void HandleCancelAll(std::unique_ptr<FlMethodResult> result) {
        JobScheduler job_scheduler;
        job_scheduler.CancelAll();

        result->Success();
    }

    void HandleBackground(const FlMethodCall& call,
                          std::unique_ptr<FlMethodResult> result) {
        LOG_DEBUG("Background call name =%s", call.method_name().c_str());

        if (call.method_name() ==
            constants::methods::kBackgroundChannelInitialized) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
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
    }
};

}  // namespace

void WorkmanagerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
    WorkmanagerTizenPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()
            ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
