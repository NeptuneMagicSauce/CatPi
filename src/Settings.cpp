#include "Settings.hpp"

#include <QSettings>
// #include <iostream>

#include "Debug.hpp"

namespace {
QSettings& instance() {
  static auto ret = QSettings{"CatPi", "CatPi"};
  return ret;
}
}  // namespace

QVariant Settings::get(const QString& key, const QVariant& defaultValue) {
  assert(Debug::Populated() == false);
  if (instance().contains(key) == false) {
    // when missing, set the default key
    // so that the instance always contains all keys
    instance().setValue(key, defaultValue);
  }
  return instance().value(key);
}

void Settings::set(const QString& key, const QVariant& value) { instance().setValue(key, value); }

QStringList Settings::keys() { return instance().allKeys(); }
