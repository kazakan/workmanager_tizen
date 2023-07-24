#include "workmanager_tizen_plugin.h"

#include <app_preference.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <job_scheduler.h>
#include <system_info.h>

#include <memory>
#include <string>

#include "extractor.h"
#include "job_scheduler_wrapper.h"
#include "log.h"
#include "sharedpreference_helper.h"

namespace {

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
    FlMethodResultRef;

class WorkmanagerTizenPlugin : public flutter::Plugin {
   public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
        auto plugin = std::make_unique<WorkmanagerTizenPlugin>();

        auto foregroundChannel =
            std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
                registrar->messenger(), constants::kForegroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

        plugin->background_channel_ =
            std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
                registrar->messenger(), constants::kBackgroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

        foregroundChannel->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                plugin_pointer->WorkmanagerHandler(call, std::move(result));
            });

        plugin->background_channel_->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                plugin_pointer->BackgroundHandler(call, std::move(result));
            });

        registrar->AddPlugin(std::move(plugin));
    }

    WorkmanagerTizenPlugin() {}

    virtual ~WorkmanagerTizenPlugin() {}

   private:
    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
        background_channel_;

    void WorkmanagerHandler(const FlMethodCall& call,
                            FlMethodResultRef result) {
        LOG_DEBUG("methodcall name %s", call.method_name().c_str());

        if (call.method_name() == constants::methods::kInitialize) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                bool isDebugMode = std::get<bool>(map[flutter::EncodableValue(
                    constants::keys::kIsInDebugModeKey)]);
                int64_t handle = std::get<int64_t>(map[flutter::EncodableValue(
                    constants::keys::kCallhandlekey)]);

                WorkmanagerTizenPlugin::InitializeHandler(
                    InitializeTask(handle, isDebugMode), std::move(result));
                return;
            }
            result->Error("WRONG_ARGS", "Wrong argument for Initialize");
            return;

        } else if (call.method_name() ==
                   constants::methods::kRegisterOneOffTask) {
            const auto& args = *call.arguments();

            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);

                bool isDebugMode = std::get<bool>(map[flutter::EncodableValue(
                    constants::keys::kIsInDebugModeKey)]);
                std::string uniqueName =
                    std::get<std::string>(map[flutter::EncodableValue(
                        constants::keys::kUniquenameKey)]);
                std::string taskname =
                    std::get<std::string>(map[flutter::EncodableValue(
                        constants::keys::kNameValueKey)]);
                std::optional<std::string> tag =
                    GetOrNullFromEncodableMap<std::string>(
                        &map, constants::keys::kTagKey);

                ExistingWorkPolicy existingWorkPolicy =
                    ExtractExistingWorkPolicyFromCall(call);
                int32_t initialDelaySeconds =
                    GetOrNullFromEncodableMap<int32_t>(
                        &map, constants::keys::kInitialDelaySecondsKey)
                        .value_or(0);
                Constraints constraintsConfig =
                    ExtractConstraintConfigFromCall(call);
                std::optional<OutOfQuotaPolicy> outOfQuotaPolicy =
                    ExtractOutOfQuotaPolicyFromCall(call);
                std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig =
                    ExtractBackoffPolicyConfigFromCall(
                        call, TaskType(TaskType::kOneOff));
                std::optional<std::string> payload =
                    GetOrNullFromEncodableMap<std::string>(
                        &map, constants::keys::kPayloadKey);

                OneOffTaskHandler(
                    OneoffTask(isDebugMode, uniqueName, taskname,
                               existingWorkPolicy, initialDelaySeconds,
                               constraintsConfig, backoffPolicyConfig,
                               outOfQuotaPolicy, tag, payload),
                    std::move(result));

                LOG_DEBUG("tag=%s payload=%s", tag.value_or("no tag").c_str(),
                          payload.value_or("no payload").c_str());
                return;
            }

            result->Error("WRONG_ARGS",
                          "Wrong argument for registerOneOffTask");
            return;
        } else if (call.method_name() ==
                   constants::methods::kRegisterPeriodicTask) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);

                bool isDebugMode = std::get<bool>(map[flutter::EncodableValue(
                    constants::keys::kIsInDebugModeKey)]);
                std::string uniqueName =
                    std::get<std::string>(map[flutter::EncodableValue(
                        constants::keys::kUniquenameKey)]);
                std::string taskname =
                    std::get<std::string>(map[flutter::EncodableValue(
                        constants::keys::kNameValueKey)]);
                std::optional<std::string> tag =
                    GetOrNullFromEncodableMap<std::string>(
                        &map, constants::keys::kTagKey);

                ExistingWorkPolicy existingWorkPolicy =
                    ExtractExistingWorkPolicyFromCall(call);
                int32_t initialDelaySeconds =
                    GetOrNullFromEncodableMap<int32_t>(
                        &map, constants::keys::kInitialDelaySecondsKey)
                        .value_or(0);
                int32_t frequencySeconds =
                    GetOrNullFromEncodableMap<int32_t>(
                        &map, constants::keys::kFrequencySecondsKey)
                        .value_or(0);

                Constraints constraintsConfig =
                    ExtractConstraintConfigFromCall(call);
                std::optional<OutOfQuotaPolicy> outOfQuotaPolicy =
                    ExtractOutOfQuotaPolicyFromCall(call);
                std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig =
                    ExtractBackoffPolicyConfigFromCall(
                        call, TaskType(TaskType::kOneOff));
                std::optional<std::string> payload =
                    GetOrNullFromEncodableMap<std::string>(
                        &map, constants::keys::kPayloadKey);

                // just for simple test
                /*
                int32_t i32;
                int64_t i64;
                if(!GetValueFromEncodableMap(&map,PeriodicTask::FREQUENCY_SECONDS_KEY,i32)){
                    LOG_ERROR("Cannot get i32");
                }

                if(!GetValueFromEncodableMap(&map,PeriodicTask::FREQUENCY_SECONDS_KEY,i64)){
                    LOG_ERROR("Cannot get i64");
                }
                */
                ////

                LOG_DEBUG("freq=%ld tag=%s payload=%s", frequencySeconds,
                          tag.value_or("no tag").c_str(),
                          payload.value_or("no payload").c_str());

                PeriodicTaskHandler(
                    PeriodicTask(isDebugMode, uniqueName, taskname,
                                 existingWorkPolicy, frequencySeconds,
                                 initialDelaySeconds, constraintsConfig,
                                 backoffPolicyConfig, outOfQuotaPolicy, tag,
                                 payload),
                    std::move(result));
                return;
            }
            result->Error("WRONG_ARGS",
                          "Wrong argument for registerPeriodicTask");
            return;
        } else if (call.method_name() ==
                   constants::methods::kCancelTaskByUniqueName) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                auto value = GetOrNullFromEncodableMap<std::string>(
                    &map, constants::keys::kCancelTaskUniqueNameKey);
                if (value.has_value()) {
                    CancelByUniqueNameHandler(
                        CancelByUniqueNameTask(value.value()),
                        std::move(result));
                    return;
                }
            }

            result->Error("WRONG_ARGS",
                          "Wrong argument for cancelTaskByUniqueName");
            return;
        } else if (call.method_name() == constants::methods::kCancelTaskByTag) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                auto value = GetOrNullFromEncodableMap<std::string>(
                    &map, constants::keys::kCancelTaskTagKey);
                if (value.has_value()) {
                    CancelByTagHandler(CancelByTagTask(value.value()),
                                       std::move(result));
                }
            }
            result->Error("WRONG_ARGS", "Wrong argument for cancelTaskBytag");
            return;
        } else if (call.method_name() == constants::methods::kCancelAllTasks) {
            CancelAllhandler(CancelTask(), std::move(result));
        }

        result->NotImplemented();
    }

   private:
    void InitializeHandler(const InitializeTask& call,
                           FlMethodResultRef result) {
        preference_set_int(constants::keys::kDispatcherHandleKey,
                           call.callback_dispathcer_handler_key_);

        result->Success();
    }

    void OneOffTaskHandler(const OneoffTask& call, FlMethodResultRef result) {
        bool initialized = false;
        preference_is_existing(constants::keys::kDispatcherHandleKey,
                               &initialized);
        if (!initialized) {
            result->Error("1", constants::kNotInitializedErrMsg,
                          flutter::EncodableValue(""));
            return;
        }

        JobScheduler jobScheduler;
        job_info_h job_info;

        LOG_DEBUG("OneOffTask name=%s", call.unique_name_.c_str());

        job_info_create(&job_info);
        jobScheduler.RegisterOneOffJob(job_info, call);

        result->Success();
    }

    void PeriodicTaskHandler(const PeriodicTask& call,
                             FlMethodResultRef result) {
        JobScheduler jobScheduler;
        job_info_h job_info;

        LOG_DEBUG("Periodictask name=%s freq=%d", call.unique_name_.c_str(),
                  call.frequency_in_seconds_);

        job_info_create(&job_info);
        jobScheduler.RegisterPeriodicJob(job_info, call);

        result->Success();
    }

    void RegisterHandler(const RegisterTask& call, FlMethodResultRef result) {}

    static void CancelByTagHandler(const CancelByTagTask& call,
                                   FlMethodResultRef result) {
        JobScheduler jobScheduler;
        jobScheduler.CancelByTag(call);

        result->Success();
    }

    void CancelByUniqueNameHandler(const CancelByUniqueNameTask& call,
                                   FlMethodResultRef result) {
        JobScheduler jobScheduler;
        jobScheduler.CancelByUniqueName(call);

        result->Success();
    }

    void CancelAllhandler(const CancelTask& call, FlMethodResultRef result) {
        // TODO : Implement
        result->Success();
    }

    void BackgroundHandler(const FlMethodCall& call, FlMethodResultRef result) {
        LOG_DEBUG("Background call name =%s", call.method_name().c_str());

        if (call.method_name() ==
            constants::methods::kBackgroundChannelInitialized) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                auto dartTask = map[flutter::EncodableValue(
                    constants::keys::kBgChannelDartTaskKey)];
                auto inputData = map[flutter::EncodableValue(
                    constants::keys::kBgChannelInputDataKey)];

                flutter::EncodableMap arg = {
                    {flutter::EncodableValue(
                         std::string(constants::keys::kBgChannelDartTaskKey)),
                     dartTask},
                    {flutter::EncodableValue(
                         std::string(constants::keys::kBgChannelInputDataKey)),
                     inputData}};

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
