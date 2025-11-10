#include <QString>
#include <QWidget>

struct Logs;
struct QDate;

struct LogsWidget : public QWidget {
  LogsWidget(Logs const& logs);
  void loadData();
  QString asAscii(const QDate& date);
};
