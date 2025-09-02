#include <QWidget>

#include "Event.hpp"

struct LogsWidget : public QWidget {
  LogsWidget();
  void update(const QList<Event>& events);
};
