/*++

  Methods for high-resolution timestamp-based code profiling.
  Useful for cases where a program might be single-threaded,
  so typical profiling tools aren't useful.

--*/

#ifndef PROFILINGTIMER_H
#define PROFILINGTIMER_H

#include <cstdint>
#include <mutex>
#include <vector>

static int mode = 0;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
inline std::uint64_t GetRawTime()
{
    static LARGE_INTEGER frequency;
    static bool frequencyInitialized = false;
    if (!frequencyInitialized)
    {
        QueryPerformanceFrequency(&frequency);
        frequencyInitialized = true;
    }
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return currentTime.QuadPart;
}
#elif defined(__APPLE__) || defined(__MACH__)
#include <mach/mach_time.h>
inline std::uint64_t GetRawTime() {
    return mach_absolute_time();
}
mode = 1;
#elif defined(__unix__) || defined(__unix) || defined(__linux__) || defined(BSD) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <ctime>
inline std::uint64_t GetRawTime() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return static_cast<std::uint64_t>(ts.tv_sec) * 1000000000 + static_cast<std::uint64_t>(ts.tv_nsec);
}
mode = 2;
#else
#include <chrono>
inline std::uint64_t GetRawTime() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}
mode = 3;
#endif

const std::uint64_t PROGRAM_START = GetRawTime();

class ProTi {
public:
    static inline std::uint64_t ReturnRawTime() noexcept {
        return GetRawTime();
    }

    static inline std::uint64_t ReturnElapsedTime() noexcept {
        return GetRawTime() - PROGRAM_START;
    }

    static inline int ReturnMode() noexcept {
		return mode;
  /*
   * mode 0 = Win32 / Win64
   * mode 1 = apple / mach
   * mode 2 = linux / bsd
   * mode 3 = chrono (no OS timer) 
   */
	}

    inline void Start() noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        timestamps.push_back(GetRawTime());
    }

    inline std::uint64_t Stop(std::size_t index) const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        std::uint64_t now = static_cast<std::uint64_t>(GetRawTime());
        return now - timestamps[index];
    }

    inline std::uint64_t ReturnTimestamp(std::size_t index) const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return timestamps[index];
    }

    inline std::size_t GetTimestampCount() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return timestamps.size();
    }

    void ClearTimestamps() noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        timestamps.clear();
    }

private:
    std::vector<std::uint64_t> timestamps;
    mutable std::mutex mutex_;
};

#endif // PROFILINGTIMER_H
