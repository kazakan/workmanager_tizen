#include "backgroundWorker.h"

#include <flutter/encodable_value.h>

#include <memory>

#include "constants.h"

void BackgroundWorker::jobStartHook(job_info_h job_info, void* userData) {
    char* job_id;
    job_info_get_job_id(job_info, &job_id);

    // payload key
    // "be.tramckrijte.workmanager.INPUT_DATA"
    // dart task key
    // "be.tramckrijte.workmanager.DART_TASK" is
    // in debug key
    // "be.tramckrijte.workmanager.IS_IN_DEBUG_MODE_KEY"
    // bg channel name
    // "be.tramckrijte.workmanager/background_channel_work_manager"
    // bg channel initialized
    // "backgroundChannelInitialized"

    // parse arguments from userData

    //

    std::string payload(static_cast<char*>(userData));

    flutter::EncodableMap arg = {
        {flutter::EncodableValue(kBgChannelInputDataKey),
         flutter::EncodableValue(payload)},
        {flutter::EncodableValue(kBgChannelDartTaskKey),
         flutter::EncodableValue(std::string(job_id))}};

    channel->InvokeMethod(kOnResultSendMethod,
                          std::make_unique<flutter::EncodableValue>(arg));
};
