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

  QMap<QString, Settings::Description> loads;
}

QVariant Settings::get(const QString& key) {
  assert(instance().contains(key));
  return instance().value(key);
}

void Settings::set(const QString& key, const QVariant& value, bool updateDebugScreen) {
  instance().setValue(key, value);

  if (updateDebugScreen) {
    Debug::changeFromOtherScreen(key);
  }
}

QStringList Settings::keys() { return instance().allKeys(); }

void Settings::load(Description description) {
  // settings must be loaded on startup
  // so that widget Debug can consume them all
  assert(Debug::Populated() == false);

  // settings should be loaded only once each
  assert(loads.contains(description.key) == false);
  loads[description.key] = description;

  if (instance().contains(description.key) == false) {
    // when missing, set the default key
    // so that the instance always contains all keys
    instance().setValue(description.key, description.defaultValue);
  }
  description.callback(instance().value(description.key));
}

const Settings::Description& Settings::Description::get(const QString& key) {
  assert(loads.contains(key));
  return loads[key];
}

bool Settings::isLoaded(const QString& key) { return loads.contains(key); }

void Settings::remove(const QString& key) { instance().remove(key); }
