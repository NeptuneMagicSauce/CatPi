#pragma once

struct ScreenBrightness {
  ScreenBrightness();

  int delayScreenSaverMinutes;

  void turnOff();
  void wakeUp();
};
