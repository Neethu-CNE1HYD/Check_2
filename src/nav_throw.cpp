#include <stdexcept>

extern "C" void crash_throw(void)
{
    throw std::runtime_error("nav: unhandled route error"); // CRASH throw: uncaught exception, SIGABRT
}
