#include "Settings.hpp"

#include <QMap>
#include <QSettings>
// #include <iostream>

#include "Debug.hpp"

namespace {
QSettings& instance() {
  static auto ret = QSettings{"CatPi", "CatPi"};
  return ret;
}

QMap<QString, Settings::Load> loads;
}  // namespace

QVariant Settings::get(const QString& key) {
  assert(instance().contains(key));
  return instance().value(key);
}

void Settings::set(const QString& key, const QVariant& value) { instance().setValue(key, value); }

QStringList Settings::keys() { return instance().allKeys(); }

void Settings::load(Load load) {
  // settings must be loaded on startup
  // so that widget Debug can consume them all
  assert(Debug::Populated() == false);

  // settings should be loaded only once each
  assert(loads.contains(load.key) == false);
  loads[load.key] = load;

  if (instance().contains(load.key) == false) {
    // when missing, set the default key
    // so that the instance always contains all keys
    instance().setValue(load.key, load.defaultValue);
  }
  load.callback(instance().value(load.key));
}

const Settings::Load& Settings::Load::get(const QString& key) {
  assert(loads.contains(key));
  return loads[key];
}
