#include "CrashHandler.hpp"

#include <cassert>
#include <cfenv>
#include <cmath>
#include <csignal>
#include <iostream>
#ifdef __x86_64__
#include <stacktrace>
#elifdef __aarch64__
#include <execinfo.h>
#endif

#include <vector>

using namespace std;

CrashHandler* CrashHandler::instance = new CrashHandler;

namespace {
  vector<int> signals() {
    static auto ret = vector<int>{SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGINT, SIGTERM};
    return ret;
  }

  function<void()> cleanUpCallback = nullptr;
  function<void(const string&, const string&)> reportCallback = nullptr;

  void handler(int signal) {
    // uninstall all handlers, if we crash again that would be looping
    for (auto inhibit : signals()) {
      std::signal(inhibit, SIG_DFL);
    }

    // call clean up
    if (cleanUpCallback != nullptr) {
      cleanUpCallback();
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
    cout << stacktrace;
    if (signal != SIGINT && reportCallback != nullptr) {
      reportCallback(name, stacktrace);
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

void CrashHandler::Test::This(CrashHandler::Test::Type type) {
  switch (type) {
    case Type::NullPtr:
      cout << *(volatile int*)nullptr;
      break;
    case Type::Assert:
      assert(false);
      break;
    case Type::StdContainerAccess:
      cout << vector<int>{}.at(0);
      break;
    case Type::DivByZeroInteger:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdivision-by-zero"
      cout << 1 / 0 << endl;
#pragma clang diagnostic pop
      break;
    case Type::FloatInf:
      [] {
        auto one = 1.0;
        cout << one / 0.0 << endl;
      }();
      break;
    case Type::FloatNaN:
      cout << -sqrt(-1.0) << endl;
      break;
    case Type::ThrowEmpty:
      throw;
    case Type::Exception:
      throw runtime_error{""};
    case Type::Terminate:
      terminate();
    case Type::Abort:
      abort();
    case Type::SigSegv:
      raise(SIGSEGV);
      break;
    case Type::SigAbort:
      raise(SIGABRT);
      break;
    case Type::SigFPE:
      raise(SIGFPE);
      break;
    case Type::SigIllFormed:
      raise(SIGILL);
      break;
    case Type::SigInterrupt:
      raise(SIGINT);
      break;
    case Type::SigTerminate:
      raise(SIGTERM);
      break;
  }
}

void CrashHandler::installCleanUpCallback(function<void()> cleanUpCallback) {
  ::cleanUpCallback = cleanUpCallback;
}

void CrashHandler::installReportCallback(
    function<void(const string&, const string&)> reportCallback) {
  ::reportCallback = reportCallback;
}
