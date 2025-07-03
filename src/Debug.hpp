#pragma once

#include <QWidget>

struct Debug : public QWidget {
  Debug();
  static bool Populated();
  void connect(std::function<void()> goBackCallback,              // back to debug screen
               std::function<void(QWidget*)> goToSettingCallback  // go to individual setting screen
  );
};
