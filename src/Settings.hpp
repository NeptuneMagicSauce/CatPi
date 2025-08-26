#include <QString>
#include <QVariant>
#include <optional>

struct Settings {
  struct Description {
    QString key;
    QString name;
    QString prompt;
    QString unit;
    QVariant defaultValue;
    std::function<void(QVariant)> callback;
    struct {
      std::optional<int> minimum = {};
      std::optional<int> maximum = {};
    } limits;

    static const Description& get(const QString& key);
  };

  static void load(Description);

  static QVariant get(const QString& key);
  static void set(const QString& key, const QVariant& value, bool updateDebugScreen = true);

  static bool isLoaded(const QString& key);  // if it exists but it's not loaded, it is obsolete
  static void remove(const QString& key);

  static QStringList keys();
};
