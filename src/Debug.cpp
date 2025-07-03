#include "Debug.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <iostream>

#include "DeltaDial.hpp"
#include "Settings.hpp"
#include "SubScreen.hpp"
#include "Widget.hpp"

struct Setting;

namespace {

auto populated = false;
struct Item {
  Setting* setting = nullptr;
  QPushButton* button = nullptr;
  SubScreen* screen = nullptr;
};
auto items = QMap<QString, Item>{};
}  // namespace

bool Debug::Populated() { return populated; }

struct Setting : public QWidget {
  const QString key;
  QLabel* description = new QLabel;
  QLabel* value = new QLabel;
  QLabel* unit = new QLabel;
  QLabel* isDefault = new QLabel;
  QToolButton* resetButton = new QToolButton;
  DeltaDial* changeButton = new DeltaDial;

  Setting(QString key) : key(key) {
    Widget::FontSized(this, 15);
    // TODO discard obsoletes:
    //   if Settings key-value is loaded from disk but not at runtime ::get()
    //   then it's an obsolete one: remove it
    //   in this Debug class, not in Settings class that does not know when it's ready
    // TODO support for callback on change
    // TODO DeltaDial: bigger maximum for smooth rotate, then divide delta
    description->setText("");  // load.description);
    value->setText("1.23456");
    unit->setText("Milliseconds");
    isDefault->setText("Yes");
    resetButton->setText("Reset");
    // changeButton

    auto layout = new QHBoxLayout;
    setLayout(layout);
    for (auto widget : QList<QWidget*>{description, value, unit, isDefault, resetButton, changeButton}) {
      layout->addWidget(widget);
    }
  }
};

Debug::Debug() {
  setStyleSheet("QWidget{font-size: 15pt;}");
  auto layout = new QGridLayout;
  setLayout(layout);
  auto const itemsPerRow = 3;
  int index = 0;
  // TODO sort keys, most interesting first
  for (auto key : Settings::keys()) {
    std::cout << "Setting: " << key.toStdString() << " = " << Settings::get(key).toString().toStdString()
              << std::endl;

    const auto& load = Settings::Load::get(key);

    auto setting = new Setting{key};
    auto button = new QPushButton{load.name};
    auto screen = new SubScreen{load.name, setting};
    items[key] = {setting, button, screen};

    auto row = index / itemsPerRow;
    auto column = index % itemsPerRow;
    layout->addWidget(button, row, column);

    ++index;
  }

  populated = true;
}

void Debug::connect(std::function<void()> goBackCallback, std::function<void(QWidget*)> goToSettingCallback) {
  for (const auto& item : items) {
    QObject::connect(item.screen->back, &QAbstractButton::released, [&] { goBackCallback(); });
    QObject::connect(item.button, &QAbstractButton::released, [&] { goToSettingCallback(item.screen); });
  }
}
