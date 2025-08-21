#include <QWidget>

#include "Logic.hpp"

struct Logs : public QWidget {
  Logs();
  void updateLogs(const QList<Logic::Event>& events);
};
