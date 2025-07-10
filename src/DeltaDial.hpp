#include <QDial>

// a dial to input small deltas, wrapping
// @member delta: the difference in value when it changed
struct DeltaDial : public QDial {
  int oldValue = 0;
  int delta = 0;
  DeltaDial();
};
