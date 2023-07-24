#ifndef FLUTTER_PLUGIN_WORKMANAGER_UTILS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_UTILS_H_

#include <flutter/encodable_value.h>

#include <optional>

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableMap *map,
                                     const char *key, T &out) {
    auto iter = map->find(flutter::EncodableValue(key));
    if (iter != map->end() && !iter->second.IsNull()) {
        if (auto *value = std::get_if<T>(&iter->second)) {
            out = *value;
            return true;
        }
    }
    return false;
}

template <typename T>
static std::optional<T> GetOrNullFromEncodableMap(
    const flutter::EncodableMap *map, const char *key) {
    auto iter = map->find(flutter::EncodableValue(key));
    if (iter != map->end() && !iter->second.IsNull()) {
        if (auto *value = std::get_if<T>(&iter->second)) {
            return *value;
        }
    }
    return std::nullopt;
}

void upper(std::string &str) {
    for (auto &c : str) c = toupper(c);
}

#endif // FLUTTER_PLUGIN_WORKMANAGER_UTILS_H_