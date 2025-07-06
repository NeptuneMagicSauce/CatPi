#include "CrashHandler.hpp"

#include <cassert>
#include <cmath>
#include <csignal>
#include <iostream>
#include <stacktrace>
#include <vector>

using namespace std;

CrashHandler* CrashHandler::instance = new CrashHandler;

namespace {
  std::function<void()> cleanUpCallback = nullptr;
  std::function<void(const std::string&, const std::string&)> reportCallback = nullptr;

  void handler(int signal) {
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
    auto stacktrace = to_string(std::stacktrace::current());
    cout << stacktrace;
    if (reportCallback != nullptr) {
      reportCallback(name, stacktrace);
    }
    exit(1);
  }

}

CrashHandler::CrashHandler() {
  // install signal handlers
  for (auto signal : {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGINT, SIGTERM}) {
    std::signal(signal, handler);
  }
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
      cout << 1.0 / 0.0 << endl;
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

void CrashHandler::installCleanUpCallback(std::function<void()> cleanUpCallback) {
  ::cleanUpCallback = cleanUpCallback;
}

void CrashHandler::installReportCallback(
    std::function<void(const std::string&, const std::string&)> reportCallback) {
  ::reportCallback = reportCallback;
}
