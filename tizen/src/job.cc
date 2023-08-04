#include "job.h"

#include <bundle.h>

#include <string>
#include <variant>

#include "extractor.h"
#include "log.h"
#include "options.h"

void AddJobInfoToBundle(bundle *bund, const JobInfo &job_info) {
    bundle_add_byte(bund, kIsInDebugMode, &job_info.is_debug_mode,
                    sizeof(bool));
    bundle_add_str(bund, kUniquename, job_info.unique_name.c_str());
    bundle_add_str(bund, kNameValue, job_info.task_name.c_str());
    bundle_add_str(bund, kTag, job_info.tag.c_str());
    bundle_add_byte(bund, kExistingWorkpolicy, &job_info.existing_work_policy,
                    sizeof(ExistingWorkPolicy));

    bundle_add_byte(bund, kInitialDelaySeconds, &job_info.initial_delay_seconds,
                    sizeof(int32_t));
    bundle_add_byte(bund, kFrequencySeconds, &job_info.frequency_seconds,
                    sizeof(int32_t));

    bundle_add_str(bund, kPayload, job_info.payload.c_str());

    bundle_add_byte(bund, kConstraintsBundle, &job_info.constraints,
                    sizeof(Constraints));
    bundle_add_byte(bund, kBackOffPolicyBundle, &job_info.backoff_policy,
                    sizeof(BackoffPolicyTaskConfig));
    bundle_add_byte(bund, kOutofQuotaPolicy, &job_info.out_of_quota_policy,
                    sizeof(OutOfQuotaPolicy));
    bundle_add_byte(bund, kIsPeriodic, &job_info.is_periodic, sizeof(bool));
}

JobInfo GetFromBundle(bundle *bund) {
    size_t size = 0;

    bool *is_debug_mode = nullptr;
    char *unique_name = nullptr;
    char *task_name = nullptr;
    char *tag = nullptr;

    ExistingWorkPolicy *existing_work_policy = nullptr;

    int32_t *initial_delay_seconds = nullptr;
    int32_t *frequency_seconds = nullptr;
    char *payload = nullptr;
    bool *is_periodic = nullptr;

    Constraints *constraints = nullptr;
    BackoffPolicyTaskConfig *backoff_policy = nullptr;
    OutOfQuotaPolicy *out_of_quota_policy = nullptr;

    bundle_get_byte(bund, kIsInDebugMode, (void **)&is_debug_mode, &size);
    bundle_get_str(bund, kUniquename, &unique_name);
    bundle_get_str(bund, kNameValue, &task_name);
    bundle_get_str(bund, kTag, &tag);
    bundle_get_byte(bund, kExistingWorkpolicy, (void **)&existing_work_policy,
                    &size);

    bundle_get_byte(bund, kInitialDelaySeconds, (void **)&initial_delay_seconds,
                    &size);
    bundle_get_byte(bund, kFrequencySeconds, (void **)&frequency_seconds,
                    &size);
    bundle_get_str(bund, kPayload, &payload);
    bundle_get_byte(bund, kIsPeriodic, (void **)&is_periodic, &size);

    bundle_get_byte(bund, kConstraintsBundle, (void **)&constraints, &size);
    bundle_get_byte(bund, kBackOffPolicyBundle, (void **)&backoff_policy,
                    &size);
    bundle_get_byte(bund, kOutofQuotaPolicy, (void **)&out_of_quota_policy,
                    &size);
    bundle_get_byte(bund, kIsPeriodic, (void **)&is_periodic, &size);

    return JobInfo(*is_debug_mode, unique_name, task_name,
                   *existing_work_policy, *initial_delay_seconds, *constraints,
                   *backoff_policy, *out_of_quota_policy, *frequency_seconds,
                   tag, payload, *is_periodic);
}
