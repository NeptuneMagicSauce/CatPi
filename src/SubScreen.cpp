#include "SubScreen.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "Widget.hpp"

namespace {
QList<SubScreen*> instances;
}

SubScreen::SubScreen(const QString& title, QWidget* contents) {
  back = new QPushButton;
  back->setSizePolicy(back->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
  static auto backIcon = QIcon{QPixmap{"://back.png"}};
  back->setIcon(backIcon);
  back->setIconSize({48, 48});

  auto topbar = [&] {
    // auto ret = new QWidget;
    auto ret = new QGroupBox;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);
    layout->addWidget(back, 1);
    auto titleLabel = new QLabel(title);
    Widget::AlignCentered(titleLabel);
    Widget::FontSized(titleLabel, 28);
    layout->addWidget(titleLabel, 4);
    ret->setMaximumHeight(80);
    return ret;
  }();

  auto layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(topbar);
  layout->addWidget(contents);

  instances.append(this);
}

void SubScreen::connect(std::function<void()> callback) {
  for (auto screen : instances) {
    QObject::connect(screen->back, &QAbstractButton::released, callback);
  }
}
