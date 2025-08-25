struct QLabel;
struct DeltaDial;
struct QAbstractButton;

#include <QWidget>
#include <optional>

struct Delay : public QWidget {
  Delay();

  DeltaDial* delayDial = nullptr;

  QAbstractButton* buttonDay = nullptr;
  QAbstractButton* buttonNight = nullptr;
  int delayDaySeconds = 0;
  int delayNightSeconds = 0;

  void setDelay(int seconds);
  void setRemaining(std::optional<int> seconds);
};
