#include <QList>

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
  Logs();

  void update(const QDateTime& now);
  void logEvent(QString const& event);
  QList<Event> const& readHistoricalData(QDate const& day) const;

  Events events;
};
