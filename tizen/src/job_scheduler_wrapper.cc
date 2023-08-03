#include "job_scheduler_wrapper.h"

#include <app_preference.h>

#include <vector>

#include "log.h"
#include "options.h"

const char* kPayloadPreferencePrefix = "WmPayload_";

JobScheduler::JobScheduler() { job_scheduler_init(); }

int JobScheduler::SetJobConstraints(job_info_h job_info,
                                    const Constraints& constraints) {
    int ret = JOB_ERROR_NONE;
    if (constraints.battery_not_low) {
        ret = job_info_set_requires_battery_not_low(job_info, true);
        if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to set job info battery_not_low: %s",
                      get_error_message(ret));
            return ret;
        }
    }

    if (constraints.charging) {
        ret = job_info_set_requires_charging(job_info, true);
        if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to set job info charging: %s",
                      get_error_message(ret));
            return ret;
        }
    }

    switch (constraints.network_type) {
        case NetworkType::kConnected:
        case NetworkType::kUnmetered:
            ret = job_info_set_requires_wifi_connection(job_info, true);
            if (ret != JOB_ERROR_NONE) {
                LOG_ERROR("Failed to set job info wifi_connection: %s",
                          get_error_message(ret));
                return ret;
            }
            break;
    }

    return ret;
}

void JobScheduler::RegisterJob(
    const bool is_debug_mode, const std::string& unique_name,
    const std::string& task_name, const ExistingWorkPolicy existing_work_policy,
    const int32_t initial_delay_seconds, const Constraints& constraints_config,
    const BackoffPolicyTaskConfig& backoff_policy_config,
    const OutOfQuotaPolicy& out_of_quota_policy, const bool isPeriodic,
    const int32_t frequency_minutes, const std::string& tag,
    const std::string& payload, job_service_callback_s* callback) {
    job_info_h job_info;
    int ret = job_info_create(&job_info);
    if (ret != JOB_ERROR_NONE) {
        LOG_ERROR("Failed to create job info: %s", get_error_message(ret));
        return;
    }

    ret = SetJobConstraints(job_info, constraints_config);
    if (ret != JOB_ERROR_NONE) {
        job_info_destroy(job_info);
        return;
    }

    if (isPeriodic) {
        ret = job_info_set_periodic(job_info, frequency_minutes);
        if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to set job info periodic: %s",
                      get_error_message(ret));
            job_info_destroy(job_info);
            return;
        }
        ret = job_info_set_persistent(job_info, true);
        if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to set job info persistent: %s",
                      get_error_message(ret));
            job_info_destroy(job_info);
            return;
        }
    } else {
        ret = job_info_set_once(job_info, true);
        if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to set job info once: %s",
                      get_error_message(ret));
            job_info_destroy(job_info);
            return;
        }
        ret = job_info_set_persistent(job_info, false);
        if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to set job non-persistent: %s",
                      get_error_message(ret));
            job_info_destroy(job_info);
            return;
        }
    }

    ret = job_scheduler_schedule(job_info, unique_name.c_str());
    if (ret != JOB_ERROR_NONE) {
        if (ret == JOB_ERROR_ALREADY_EXIST) {
            switch (existing_work_policy) {
                case ExistingWorkPolicy::kReplace:
                case ExistingWorkPolicy::kUpdate:
                    CancelByUniqueName(unique_name);
                    ret = job_scheduler_schedule(job_info, unique_name.c_str());
                    if (ret != JOB_ERROR_NONE) {
                        LOG_ERROR("Failed to schedule job: %s",
                                  get_error_message(ret));
                    } else {
                        SavePayload(unique_name, payload);
                        if (!callback) {
                            break;
                        }
                        SetCallback(unique_name.c_str(), callback);
                    }
                    break;
                default:
                    LOG_INFO("Job already exists but ignored. Job name: %s",
                             unique_name.c_str());
            }
        } else {
            LOG_ERROR("Failed to schedule job: %s", get_error_message(ret));
        }
    } else {
        SavePayload(unique_name, payload);
        if (callback) {
            SetCallback(unique_name.c_str(), callback);
        }
    }

    job_info_destroy(job_info);
}

void JobScheduler::CancelByTag(const std::string& task) {
    // Not supported
}

void JobScheduler::CancelByUniqueName(const std::string& name) {
    int ret = job_scheduler_cancel(name.c_str());
    if (ret != JOB_ERROR_NONE) {
        LOG_ERROR("Failed to cancel job with name %s: %s", name.c_str(),
                  get_error_message(ret));
        return;
    }
    preference_remove(GetPayloadKey(name).c_str());
}

void JobScheduler::CancelAll() {
    auto job_names = GetAllJobs();

    job_scheduler_cancel_all();

    for (const auto name : job_names) {
        preference_remove(GetPayloadKey(name).c_str());
    }
}

job_service_h JobScheduler::SetCallback(const char* job_name,
                                        job_service_callback_s* callback) {
    job_service_h service = nullptr;
    int ret = job_scheduler_service_add(job_name, callback, nullptr, &service);
    if (ret != JOB_ERROR_NONE) {
        LOG_ERROR("Failed to add service to job: %s", get_error_message(ret));
        return nullptr;
    }
    return service;
}

std::vector<std::string> JobScheduler::GetAllJobs() {
    std::vector<std::string> jobs;

    int code = job_scheduler_foreach_job(
        [](job_info_h job_info, void* user_data) {
            char* job_id = NULL;
            auto vec = static_cast<std::vector<std::string>*>(user_data);

            job_info_get_job_id(job_info, &job_id);
            vec->emplace_back(job_id);
            return true;
        },
        &jobs);

    return jobs;
}

void JobScheduler::SavePayload(const std::string& job_name,
                               const std::string& payload) {
    const std::string payload_key = GetPayloadKey(job_name).c_str();
    preference_set_string(payload_key.c_str(), payload.c_str());
}

std::string JobScheduler::GetPayloadKey(const std::string& job_name) {
    return kPayloadPreferencePrefix + job_name;
}