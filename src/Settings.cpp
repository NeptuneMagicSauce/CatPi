#include "Settings.hpp"

#include <QSettings>

namespace {
QSettings& instance() {
  static auto ret = QSettings{"CatPi", "CatPi"};
  return ret;
}
}  // namespace

int Settings::getInt(const QString& key, int defaultValue) {
  if (instance().contains(key) == false) {
    // when missing, set the default key
    // so that the instance always contains all keys
    instance().setValue(key, defaultValue);
  }
  return instance().value(key).toInt();
}
void Settings::setInt(const QString& key, int value) { instance().setValue(key, value); }

double Settings::getDouble(const QString& key, double defaultValue) {
  if (instance().contains(key) == false) {
    instance().setValue(key, defaultValue);
  }
  return instance().value(key).toDouble();
}

void Settings::setDouble(const QString& key, double value) { instance().setValue(key, value); }
