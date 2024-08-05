/*++

  Methods for high-resolution timestamp-based code profiling.
  Useful for cases where a program might be single-threaded,
  so typical profiling tools aren't useful.

--*/

#ifndef PROFILINGTIMER_H
#define PROFILINGTIMER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*
mode 0 = Win32 / Win64
mode 1 = apple / mach
mode 2 = linux / bsd
mode 3 = chrono (no OS timer)
*/

static int mode = 0;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
uint64_t GetRawTime() {
    static LARGE_INTEGER frequency;
    static int frequencyInitialized = 0;
    if (!frequencyInitialized) {
        QueryPerformanceFrequency(&frequency);
        frequencyInitialized = 1;
    }
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return currentTime.QuadPart;
}
#elif defined(__APPLE__) || defined(__MACH__)
#include <mach/mach_time.h>
uint64_t GetRawTime() {
    return mach_absolute_time();
}
mode = 1;
#elif defined(__unix__) || defined(__unix) || defined(__linux__) || defined(BSD) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <time.h>
uint64_t GetRawTime() {
    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
}
mode = 2;
#else
#include <chrono>
uint64_t GetRawTime() {
    return (uint64_t)std::chrono::steady_clock::now().time_since_epoch().count();
}
mode = 3;
#endif

const uint64_t PROGRAM_START = GetRawTime();

typedef struct {
    uint64_t* timestamps;
    size_t count;
    size_t capacity;
    pthread_mutex_t mutex;
} ProTi;

void ProTi_Init(ProTi* timer) {
    timer->timestamps = NULL;
    timer->count = 0;
    timer->capacity = 0;
    pthread_mutex_init(&timer->mutex, NULL);
}

void ProTi_Destroy(ProTi* timer) {
    free(timer->timestamps);
    pthread_mutex_destroy(&timer->mutex);
}

uint64_t ProTi_ReturnRawTime() {
    return GetRawTime();
}

uint64_t ProTi_ReturnElapsedTime() {
    return GetRawTime() - PROGRAM_START;
}

int ProTi_ReturnMode() {
    return mode;
}

void ProTi_Start(ProTi* timer) {
    pthread_mutex_lock(&timer->mutex);
    if (timer->count == timer->capacity) {
        timer->capacity = timer->capacity ? timer->capacity * 2 : 1;
        timer->timestamps = realloc(timer->timestamps, timer->capacity * sizeof(uint64_t));
    }
    timer->timestamps[timer->count++] = GetRawTime();
    pthread_mutex_unlock(&timer->mutex);
}

uint64_t ProTi_Stop(ProTi* timer, size_t index) {
    pthread_mutex_lock(&timer->mutex);
    uint64_t now = GetRawTime();
    uint64_t elapsed = now - timer->timestamps[index];
    pthread_mutex_unlock(&timer->mutex);
    return elapsed;
}

uint64_t ProTi_ReturnTimestamp(ProTi* timer, size_t index) {
    pthread_mutex_lock(&timer->mutex);
    uint64_t timestamp = timer->timestamps[index];
    pthread_mutex_unlock(&timer->mutex);
    return timestamp;
}

size_t ProTi_GetTimestampCount(ProTi* timer) {
    pthread_mutex_lock(&timer->mutex);
    size_t count = timer->count;
    pthread_mutex_unlock(&timer->mutex);
    return count;
}

void ProTi_ClearTimestamps(ProTi* timer) {
    pthread_mutex_lock(&timer->mutex);
    timer->count = 0;
    pthread_mutex_unlock(&timer->mutex);
}

#endif // PROFILINGTIMER_H
