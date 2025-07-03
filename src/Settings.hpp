#pragma once

#include <QString>
#include <QVariant>

struct Settings {
  struct Load {
    QString key;
    QString name;
    QString description;
    QString unit;
    QVariant defaultValue;

    static const Load& get(const QString& key);
  };

  static QVariant load(Load);
  static QVariant get(const QString& key);
  static void set(const QString& key, const QVariant& value);

  static QStringList keys();
};
