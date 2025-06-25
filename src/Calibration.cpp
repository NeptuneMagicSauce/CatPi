#include "Calibration.hpp"

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include "MainWindow.hpp"

struct CalibrationImpl {
  CalibrationImpl(Calibration*);
};

// auto spacer = []() {
//   auto spacer = new QWidget;
//   spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//   return spacer;
// };

Calibration::Calibration() {  // : impl() { new CalibrationImpl(this)) {

  setStyleSheet("QWidget{font-size: 20pt; } ");

  buttons.step1 = new QPushButton("Step 1");
  buttons.step2 = new QPushButton("Step 2");
  buttons.back = new QToolButton;
  buttons.back->setText("Back");

  buttons.back->setIcon(MainWindow::StandardIcon(QStyle::StandardPixmap::SP_MediaSeekBackward));

  auto topbar = [&]() {
    auto ret = new QWidget;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);
    layout->addWidget(buttons.back);
    auto title = new QLabel("  Calibration");
    title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(title);
    return ret;
  }();

  auto central = [&]() {
    auto ret = new QWidget;
    auto layout = new QVBoxLayout;
    ret->setLayout(layout);
    layout->addWidget(buttons.step1);
    layout->addWidget(buttons.step2);
    ret->setSizePolicy(ret->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    return ret;
  }();

  auto layout = new QVBoxLayout();
  setLayout(layout);

  layout->addWidget(topbar);
  layout->addWidget(central);
}

void Calibration::Callbacks::step1() {}
void Calibration::Callbacks::step2() {}
