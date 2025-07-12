#include "ProtectedButton.hpp"

#include <QBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

struct ProtectedButtonImpl {
  ProtectedButtonImpl(QTimer& finished, QPushButton& button);

  QProgressBar progress;  // won't this fail beceause QT wants to destroy it?
  QTimer pressedTimer;
  QTimer& finished;
  QPushButton& button;
  int buttonPressedTicks = 0;
  static const int maxTicks = 40;
  static const int interval = 20;
};

ProtectedButton::~ProtectedButton() { qDebug() << Q_FUNC_INFO << "not implemented"; }

ProtectedButton::ProtectedButton() {
  finished = new QTimer;
  button = new QPushButton;
  impl = new ProtectedButtonImpl(*finished, *button);
  auto layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(&impl->progress);
  layout->addWidget(&impl->button);
}

ProtectedButtonImpl::ProtectedButtonImpl(QTimer& finished, QPushButton& button)
    : finished(finished), button(button) {
  pressedTimer.setSingleShot(true);
  pressedTimer.setInterval(interval);

  finished.setInterval(0);
  finished.setSingleShot(true);

  progress.setMaximum(maxTicks);
  progress.setTextVisible(false);

  // make transparent so that it's not visible when disabled
  progress.setStyleSheet("background-color: rgba(255,0,0,0)");

  progress.setEnabled(false);

  progress.setMaximumHeight(15);
  progress.setSizePolicy({QSizePolicy::Policy::Minimum, progress.sizePolicy().verticalPolicy()});

  // Vertical progress bar:
  // progress.setOrientation(Qt::Vertical);
  // progress.setMaximumWidth(15);
  // // progress.setMaximumHeight(100); bad
  // progress.setSizePolicy({button.sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Minimum});

  QObject::connect(&button, &QAbstractButton::pressed, [this]() {
    buttonPressedTicks = 0;
    progress.setValue(0);
    progress.setEnabled(true);
    pressedTimer.start();
  });
  QObject::connect(&button, &QAbstractButton::released, [this]() {
    pressedTimer.stop();
    progress.setValue(0);
    progress.setEnabled(false);
  });
  QObject::connect(&pressedTimer, &QTimer::timeout, [&]() {
    ++buttonPressedTicks;
    progress.setValue(buttonPressedTicks);
    if (buttonPressedTicks < maxTicks) {
      pressedTimer.start();
    } else {
      // qDebug() << this << "long press";
      progress.setValue(0);
      progress.setEnabled(false);
      finished.start();
    }
  });
}
