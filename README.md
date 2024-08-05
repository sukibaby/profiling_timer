This is my timer class designed for profiling single-threaded applications with high resolution timestamps.

I've provided ready-to-use classes for both C and C++.

Here is an example of how you might use the `ProTi` (PROfiling TImer) class in your C++ project. The idea is you can set a bunch of timestamps, figure out how many you have (or what the index of each one is), figure out what timer is being used by the system, etc.

```
ProTi profiler;

// Set a bunch of timestamps and figure out what each of their indexes are
profiler.Start();
profiler.GetTimestampCount();
profiler.Start();
profiler.GetTimestampCount();
profiler.Start();
profiler.GetTimestampCount();

// Code block to be profiled
exampleFunction();

// Stop the first timer and get the elapsed time
std::uint64_t elapsedTime = profiler.Stop(0);

// Print the elapsed time in nanoseconds
std::cout << "Elapsed time: " << elapsedTime << " nanoseconds" << std::endl;

// Print the mode used for timing
std::cout << "Timing mode: " << profiler.ReturnMode() << std::endl;
```
