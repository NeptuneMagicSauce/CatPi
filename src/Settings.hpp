#pragma once

struct QString;
#include <QVariant>

struct Settings {
  static QVariant get(const QString& key, const QVariant& defaultValue);
  static void set(const QString& key, const QVariant& value);
};
