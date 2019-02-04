#pragma once
#if defined __MINGW32__ && !(defined _GLIBCXX_HAS_GTHREADS)
#include <mingw-std-threads/mingw.thread.h>
#include <mingw-std-threads/mingw.mutex.h>
#include <mingw-std-threads/mingw.condition_variable.h>
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif
