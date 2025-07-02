#pragma once

#include <QWidget>

struct Debug : public QWidget {
  Debug();
  static bool Populated();
  void connect(std::function<void()> goBackCallback, std::function<void(QWidget*)> goToSettingCallback);
};
