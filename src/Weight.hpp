struct QTimer;

#include <QGroupBox>
#include <QString>
#include <optional>

class Weight : public QGroupBox {
 public:
  Weight();

  const QString messageFinished;
  QTimer* eventTareFinished();

  void update(std::optional<double> value);
  double tare();
  std::optional<double> weightTarred();
};
