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
#include "fltypes.h"
#include "job_scheduler_wrapper.h"
#include "log.h"
#include "sharedpreference_helper.h"

namespace {

class WorkmanagerTizenPlugin : public flutter::Plugin {
   public:
    static std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
        backgroundChannel;

    static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
        auto foregroundChannel =
            std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
                registrar->messenger(), kForegroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

        backgroundChannel =
            std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
                registrar->messenger(), kBackgroundChannelName,
                &flutter::StandardMethodCodec::GetInstance());

        auto plugin = std::make_unique<WorkmanagerTizenPlugin>();

        foregroundChannel->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                WorkmanagerHandler(call, std::move(result));
            });

        backgroundChannel->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                BackgroundHandler(call, std::move(result));
            });

        registrar->AddPlugin(std::move(plugin));
    }

    WorkmanagerTizenPlugin() {}

    virtual ~WorkmanagerTizenPlugin() {}

    static void WorkmanagerHandler(const FlMethodCall& call,
                                   FlMethodResultRef result) {
        LOG_DEBUG("methodcall name %s", call.method_name().c_str());

        if (call.method_name() == kInitialize) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                bool isDebugMode = std::get<bool>(
                    map[flutter::EncodableValue(kIsInDebugModeKey)]);
                int64_t handle = std::get<int64_t>(
                    map[flutter::EncodableValue(kCallhandlekey)]);

                WorkmanagerTizenPlugin::InitializeHandler(
                    InitializeTask(handle, isDebugMode), std::move(result));
                return;
            }
            result->Error("WRONG_ARGS", "Wrong argument for Initialize");
            return;

        } else if (call.method_name() == kRegisterOneOffTask) {
            const auto& args = *call.arguments();

            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);

                bool isDebugMode = std::get<bool>(
                    map[flutter::EncodableValue(kIsInDebugModeKey)]);
                std::string uniqueName = std::get<std::string>(
                    map[flutter::EncodableValue(kUniquenameKey)]);
                std::string taskname = std::get<std::string>(
                    map[flutter::EncodableValue(kNameValueKey)]);
                std::optional<std::string> tag =
                    GetOrNullFromEncodableMap<std::string>(&map, kTagKey);

                ExistingWorkPolicy existingWorkPolicy =
                    extractExistingWorkPolicyFromCall(call);
                int32_t initialDelaySeconds =
                    GetOrNullFromEncodableMap<int32_t>(&map,
                                                       kInitialDelaySecondsKey)
                        .value_or(0);
                Constraints constraintsConfig =
                    extractConstraintConfigFromCall(call);
                std::optional<OutOfQuotaPolicy> outOfQuotaPolicy =
                    extractOutOfQuotaPolicyFromCall(call);
                std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig =
                    extractBackoffPolicyConfigFromCall(
                        call, TaskType(TaskType::ONE_OFF));
                std::optional<std::string> payload =
                    GetOrNullFromEncodableMap<std::string>(&map, kPayloadKey);

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
        } else if (call.method_name() == kRegisterPeriodicTask) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);

                bool isDebugMode = std::get<bool>(
                    map[flutter::EncodableValue(kIsInDebugModeKey)]);
                std::string uniqueName = std::get<std::string>(
                    map[flutter::EncodableValue(kUniquenameKey)]);
                std::string taskname = std::get<std::string>(
                    map[flutter::EncodableValue(kNameValueKey)]);
                std::optional<std::string> tag =
                    GetOrNullFromEncodableMap<std::string>(&map, kTagKey);

                ExistingWorkPolicy existingWorkPolicy =
                    extractExistingWorkPolicyFromCall(call);
                int32_t initialDelaySeconds =
                    GetOrNullFromEncodableMap<int32_t>(&map,
                                                       kInitialDelaySecondsKey)
                        .value_or(0);
                int32_t frequencySeconds = GetOrNullFromEncodableMap<int32_t>(
                                               &map, kFrequencySecondsKey)
                                               .value_or(0);

                Constraints constraintsConfig =
                    extractConstraintConfigFromCall(call);
                std::optional<OutOfQuotaPolicy> outOfQuotaPolicy =
                    extractOutOfQuotaPolicyFromCall(call);
                std::optional<BackoffPolicyTaskConfig> backoffPolicyConfig =
                    extractBackoffPolicyConfigFromCall(
                        call, TaskType(TaskType::ONE_OFF));
                std::optional<std::string> payload =
                    GetOrNullFromEncodableMap<std::string>(&map, kPayloadKey);

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
        } else if (call.method_name() == kCancelTaskByUniqueName) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                auto value = GetOrNullFromEncodableMap<std::string>(
                    &map, kCancelTaskUniqueNameKey);
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
        } else if (call.method_name() == kCancelTaskByTag) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                auto value = GetOrNullFromEncodableMap<std::string>(
                    &map, kCancelTaskTagKey);
                if (value.has_value()) {
                    CancelByTagHandler(CancelByTagTask(value.value()),
                                       std::move(result));
                }
            }
            result->Error("WRONG_ARGS", "Wrong argument for cancelTaskBytag");
            return;
        } else if (call.method_name() == kCancelAllTasks) {
            CancelAllhandler(CancelTask(), std::move(result));
        }

        result->NotImplemented();
    }

   private:
    static void InitializeHandler(const InitializeTask& call,
                                  FlMethodResultRef result) {
        preference_set_int(DISPATHCER_HANDLE_KEY,
                           call.callBackDispathcerHandlerKey);

        result->Success();
    }

    static void OneOffTaskHandler(const OneoffTask& call,
                                  FlMethodResultRef result) {
        bool initialized = false;
        preference_is_existing(DISPATHCER_HANDLE_KEY, &initialized);
        if (!initialized) {
            result->Error("1", kNotInitializedErrMsg,
                          flutter::EncodableValue(""));
            return;
        }

        JobScheduler jobScheduler;
        job_info_h job_info;

        LOG_DEBUG("OneOffTask name=%s", call.uniquename.c_str());

        job_info_create(&job_info);
        jobScheduler.registerOneOffJob(job_info, call);

        result->Success();
    }

    static void PeriodicTaskHandler(const PeriodicTask& call,
                                    FlMethodResultRef result) {
        JobScheduler jobScheduler;
        job_info_h job_info;

        LOG_DEBUG("Periodictask name=%s freq=%d", call.uniquename.c_str(),
                  call.frequencyInSeconds);

        job_info_create(&job_info);
        jobScheduler.registerPeriodicJob(job_info, call);

        result->Success();
    }

    static void RegisterHandler(const RegisterTask& call,
                                FlMethodResultRef result) {}

    static void CancelByTagHandler(const CancelByTagTask& call,
                                   FlMethodResultRef result) {
        JobScheduler jobScheduler;
        jobScheduler.cancelByTag(call);

        result->Success();
    }

    static void CancelByUniqueNameHandler(const CancelByUniqueNameTask& call,
                                          FlMethodResultRef result) {
        JobScheduler jobScheduler;
        jobScheduler.cancelByUniqueName(call);

        result->Success();
    }

    static void CancelAllhandler(const CancelTask& call,
                                 FlMethodResultRef result) {
        result->Success();
    }

    static void BackgroundHandler(const FlMethodCall& call,
                                  FlMethodResultRef result) {
        LOG_DEBUG("Background call name =%s", call.method_name().c_str());

        if (call.method_name() == kBackgroundChannelInitialized) {
            const auto& args = *call.arguments();
            if (std::holds_alternative<flutter::EncodableMap>(args)) {
                flutter::EncodableMap map =
                    std::get<flutter::EncodableMap>(args);
                auto dartTask =
                    map[flutter::EncodableValue(kBgChannelDartTaskKey)];
                auto inputData =
                    map[flutter::EncodableValue(kBgChannelInputDataKey)];

                flutter::EncodableMap map = {
                    {flutter::EncodableValue(
                         std::string(kBgChannelDartTaskKey)),
                     dartTask},
                    {flutter::EncodableValue(
                         std::string(kBgChannelInputDataKey)),
                     inputData}};

                backgroundChannel->InvokeMethod(
                    kOnResultSendMethod,
                    std::make_unique<flutter::EncodableValue>(map));
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
