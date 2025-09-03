#include <QWidget>

struct Logs;

struct LogsWidget : public QWidget {
  LogsWidget(Logs const& logs);
  void loadData();
};
