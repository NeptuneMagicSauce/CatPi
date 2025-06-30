#pragma once

struct QString;

struct Settings {
  static int getInt(const QString& key, int defaultValue);
  static void setInt(const QString& key, int value);

  static double getDouble(const QString& key, double defaultValue);
  static void setDouble(const QString& key, double value);
};
