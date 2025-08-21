#include <QWidget>

#include "Event.hpp"

struct Logs : public QWidget {
  Logs();
  void updateLogs(const QList<Event>& events);
};
