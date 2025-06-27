#pragma once

#include <QSettings>

struct Settings {
  static QSettings& instance();
};
