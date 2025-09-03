#include <QWidget>

#include "Events.hpp"

struct LogsSmallWidget : public QWidget {
  LogsSmallWidget();
  void update(const QList<Event>& events);
};
