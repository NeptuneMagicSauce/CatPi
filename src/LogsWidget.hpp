#include <QWidget>

#include "Events.hpp"

struct LogsWidget : public QWidget {
  LogsWidget();
  void update(const QList<Event>& events);
};
