#include "Events.hpp"

struct QString;
struct QDateTime;

struct Logs {
  Logs();

  void update(const QDateTime& now);
  void logEvent(QString const& event);  // TODO rename

  Events events;
};
