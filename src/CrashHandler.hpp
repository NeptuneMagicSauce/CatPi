#pragma once

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
  CrashHandler();
};
