#include <QByteArray>

struct QString;

struct Compressor {
  static void Do(QString const& pathSource, QString& pathDest);
  [[nodiscard]] static bool Exists(QString const& path);
  [[nodiscard]] static QByteArray Read(QString const& path);
};
