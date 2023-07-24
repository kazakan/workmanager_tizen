#ifndef FLUTTER_PLUGIN_WORKMANAGER_BACKGROUNDWORKER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_BACKGROUNDWORKER_H_

#include <flutter/method_call.h>
#include <flutter/method_channel.h>
#include <job_scheduler.h>

#include <memory>

#include "options.h"

class BackgroundWorker {
    static std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
        channel;

    static void jobStartHook(job_info_h job_info, void* userData);
};

#endif