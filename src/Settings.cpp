#include "Settings.hpp"

#include <QSettings>
#include <iostream>

namespace {
QSettings& instance() {
  static auto ret = QSettings{"CatPi", "CatPi"};
  return ret;
}
}  // namespace

QVariant Settings::get(const QString& key, const QVariant& defaultValue) {
  if (instance().contains(key) == false) {
    // when missing, set the default key
    // so that the instance always contains all keys
    instance().setValue(key, defaultValue);
  }
  std::cout << "Setting: " << key.toStdString() << " = " << instance().value(key).toString().toStdString()
            << std::endl;
  return instance().value(key);
}

void Settings::set(const QString& key, const QVariant& value) { instance().setValue(key, value); }
