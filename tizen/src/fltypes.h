#ifndef FLUTTER_PLUGIN_WORKMANAGER_FLTYPES_H_
#define FLUTTER_PLUGIN_WORKMANAGER_FLTYPES_H_

#include <flutter/method_call.h>
#include <flutter/method_channel.h>

using FlMethodCall = flutter::MethodCall<flutter::EncodableValue>;
using FlMethodResultRef =
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>;

#endif // FLUTTER_PLUGIN_WORKMANAGER_FLTYPES_H_