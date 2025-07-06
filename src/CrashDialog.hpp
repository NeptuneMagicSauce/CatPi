#pragma once

#include <QDialog>
struct QString;

struct CrashTester : public QDialog {
  CrashTester();
};

struct CrashDialog {
  static void ShowStackTrace(const QString& error, const QString& stack);
};
