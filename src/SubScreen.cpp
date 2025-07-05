#include "SubScreen.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
// #include <iostream>

#include "Widget.hpp"

namespace {
  QList<SubScreen*> instances;
}

SubScreen::SubScreen(const QString& title, QWidget* contents) : contents(contents) {
  auto topbar = [&] {
    auto ret = new QGroupBox;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);

    back = new QPushButton;
    back->setSizePolicy(back->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    static auto backIcon = QIcon{QPixmap{"://back.png"}};
    back->setIcon(backIcon);
    back->setIconSize({48, 48});
    layout->addWidget(back, 1);

    auto titleLabel = new QLabel(title);
    titleLabel->setMaximumWidth(580);  // otherwise it does not fit
    Widget::AlignCentered(titleLabel);
    Widget::FontSized(titleLabel, 28);
    layout->addWidget(titleLabel, 4);
    ret->setMaximumHeight(80);
    return ret;
  };

  auto layout = new QVBoxLayout;
  setLayout(layout);
  if (title.isEmpty() == false) {
    layout->addWidget(topbar());
  }
  layout->addWidget(contents);

  instances.append(this);
}

void SubScreen::connect(std::function<void()> callback) {
  for (auto screen : instances) {
    if (screen->back == nullptr) {
      continue;
    }
    QObject::connect(screen->back, &QAbstractButton::released, callback);
  }
}
