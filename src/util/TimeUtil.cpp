#include "TimeUtil.h"

#ifdef _WIN32
    #include <windows.h>
    uint32_t TimeUtil::getTimeNowMs() {
        LARGE_INTEGER frequency, counter;
        // Get the frequency of the high-resolution counter
        QueryPerformanceFrequency(&frequency);
        // Get the current value of the high-resolution counter
        QueryPerformanceCounter(&counter);
        // Return the time in milliseconds
        return (double)counter.QuadPart / (double)frequency.QuadPart * 1000.0;
    }
#elif
    #include <time.h>
    uint32_t TimeUtil::getTimeNowMs() {
        timespec now = {};
        clock_gettime( CLOCK_MONOTONIC_RAW, &now );
        auto now_ms = (uint32_t)((uint64_t)now.tv_sec * UINT64_C(1000) + (uint64_t)now.tv_nsec/1000000);
        return now_ms;
    }
#endif //_WIN32
