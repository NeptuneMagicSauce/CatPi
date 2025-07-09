#pragma once

#include <functional>
#include <string>
#include <vector>

struct CrashHandler {
  static CrashHandler* instance;

  std::function<void()> cleanUpCallback = nullptr;
  std::function<void(const std::string&, const std::string&)> reportCallback = nullptr;

 private:
  CrashHandler();
};
