#include <functional>

struct ScreenBrightness {
  ScreenBrightness();

  void setCallbackOnChange(std::function<void(bool)> onChangeCallback);

  static void reset();
};
