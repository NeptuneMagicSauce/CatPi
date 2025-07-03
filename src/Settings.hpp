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
    std::function<void(QVariant)> callback = [](QVariant) {};  // test if this is needed

    static const Load& get(const QString& key);
  };

  static void load(Load);
  static QVariant get(const QString& key);
  static void set(const QString& key, const QVariant& value, bool updateDebugScreen = true);

  static QStringList keys();
};
