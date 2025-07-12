struct QLabel;
struct DeltaDial;
#include <QGroupBox>

struct Delay : public QGroupBox {
  Delay();

  DeltaDial* delayDial = nullptr;

  void setDelay(int seconds);
  void setRemaining(int seconds);
};
