#define AssertSingleton()     \
  {                           \
    static auto first = true; \
    assert(first);            \
    first = false;            \
  }
