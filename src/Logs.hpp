#include <QList>
#include <QTimer>

#include "Events.hpp"

struct QString;
struct QDateTime;
struct QDate;

struct Logs {
  struct Event {
    enum struct Type {  //
      Boot,
      DispenseAuto,
      DispenseManual,
      Eat,
    };
    Type type;
    QDateTime time;
    double weight;

    static const QSet<Type> dispenseTypes;
  };
  Logs(QString const& dataDirectory);

  QTimer eventLogFileChanged;

  void update(const QDateTime& now);
  void logEvent(QString const& event);
  [[nodiscard]] QList<Event> const& readHistoricalData(QDate const& day) const;
  [[nodiscard]] bool hasHistoricalData(QDate const& day) const;
  [[nodiscard]] QString dateToFilePath(const QDate& date) const;

  Events events;
};
