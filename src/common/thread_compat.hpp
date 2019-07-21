#pragma once
#if defined __MINGW32__ && !(defined _GLIBCXX_HAS_GTHREADS)
#include <mingw.thread.h>
#include <mingw.mutex.h>
#include <mingw.condition_variable.h>
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif
