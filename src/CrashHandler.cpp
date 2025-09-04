#include "CrashHandler.hpp"

#include <cassert>
#include <cfenv>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#ifdef __x86_64__
#include <stacktrace>
#elifdef __aarch64__
#include <execinfo.h>
#endif

using namespace std;

CrashHandler* CrashHandler::instance = new CrashHandler;

namespace {
  const std::vector<int>& signals() {
    static auto ret = vector<int>{SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGINT, SIGTERM};
    return ret;
  }

  void handler(int signal) {
    // uninstall all handlers, if we crash again that would be looping
    for (auto inhibit : signals()) {
      std::signal(inhibit, SIG_DFL);
    }

    // call clean up
    if (CrashHandler::instance->cleanUpCallback != nullptr) {
      CrashHandler::instance->cleanUpCallback();
    }

    auto name = signal == SIGSEGV   ? "SIGSEGV"
                : signal == SIGABRT ? "SIGABRT"
                : signal == SIGFPE  ? "SIGFPE"
                : signal == SIGILL  ? "SIGILL"
                : signal == SIGINT  ? "SIGINT"
                : signal == SIGTERM ? "SIGTERM"
                                    : "??";
    cout << "Signal " << name << endl;

#ifdef __x86_64__
    auto stacktrace = to_string(stacktrace::current());
#elifdef __aarch64__
    auto stacktrace = string{};
    auto const size = 256;
    void* callstack[size];
    auto frames = backtrace(callstack, size);
    auto strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames; ++i) {
      stacktrace += string{strs[i]} + "\n";
    }
#else
#error "not supported"
#endif
    try {
      std::ostringstream logfilename;
      auto homeDir = getenv("HOME");
      if (homeDir != nullptr) {
        logfilename << homeDir << "/";
      }
      auto t = std::time(nullptr);
      logfilename << "crashlog-" << std::put_time(std::localtime(&t), "%d-%m-%Y-%Hh%Mm%Ss")
                  << ".txt";
      ofstream fout{logfilename.str()};
      fout << stacktrace;
    } catch (exception&) {
    }
    cout << stacktrace;
    if (signal != SIGINT && CrashHandler::instance->reportCallback != nullptr) {
      CrashHandler::instance->reportCallback(name, stacktrace);
    }
    exit(1);
  }
}

CrashHandler::CrashHandler() {
  // install signal handlers
  for (auto enable : signals()) {
    std::signal(enable, handler);
  }

  // trap on INF and NAN
  feenableexcept(FE_DIVBYZERO | FE_INVALID);
}
