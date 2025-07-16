struct QLabel;
struct DeltaDial;
struct QAbstractButton;

#include <QGroupBox>

struct Delay : public QGroupBox {
  Delay();

  DeltaDial* delayDial = nullptr;

  QAbstractButton* buttonDay = nullptr;
  QAbstractButton* buttonNight = nullptr;
  int delayDaySeconds = 0;
  int delayNightSeconds = 0;

  void setDelay(int seconds);
  void setRemaining(int seconds);
};
