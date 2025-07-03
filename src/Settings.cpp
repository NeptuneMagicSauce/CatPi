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

QVariant Settings::get(const QString& key) {
  assert(instance().contains(key));
  return instance().value(key);
}

void Settings::set(const QString& key, const QVariant& value) { instance().setValue(key, value); }

QStringList Settings::keys() { return instance().allKeys(); }

QVariant Settings::load(Load load) {
  // settings must be loaded on startup
  // so that widget Debug can consume them all
  assert(Debug::Populated() == false);

  if (instance().contains(load.key) == false) {
    // when missing, set the default key
    // so that the instance always contains all keys
    instance().setValue(load.key, load.defaultValue);
  }
  return instance().value(load.key);
}
