#pragma once

#include <functional>
#include <string>

struct CrashHandler {
  struct Test {
    enum struct Type {
      NullPtr,
      Assert,
      StdContainerAccess,
      DivByZeroInteger,
      FloatInf,
      FloatNaN,
      ThrowEmpty,
      Exception,
      Terminate,
      Abort,
      SigSegv,
      SigAbort,
      SigFPE,
      SigIllFormed,
      SigInterrupt,
      SigTerminate,
    };
    static void This(Type);
  };

  static CrashHandler* instance;

  std::function<void()> cleanUpCallback = nullptr;
  std::function<void(const std::string&, const std::string&)> reportCallback = nullptr;

  CrashHandler();
};
