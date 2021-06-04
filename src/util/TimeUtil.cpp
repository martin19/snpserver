#include "TimeUtil.h"

#include <time.h>

uint32_t TimeUtil::getTimeNowMs() {
    timespec now = {};
    clock_gettime( CLOCK_MONOTONIC_RAW, &now );
    auto now_ms = (uint32_t)((uint64_t)now.tv_sec * UINT64_C(1000) + (uint64_t)now.tv_nsec/1000000);
    return now_ms;
}
