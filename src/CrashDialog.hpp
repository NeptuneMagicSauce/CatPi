struct QString;

struct CrashDialog {
  static void ShowStackTrace(const QString& error, const QString& stack);
};
