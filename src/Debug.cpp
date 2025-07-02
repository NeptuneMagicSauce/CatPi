#include "Debug.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <iostream>

#include "Settings.hpp"
#include "SubScreen.hpp"

namespace {

auto populated = false;
auto buttons = QMap<QString, QPushButton*>{};
auto screens = QMap<QString, SubScreen*>{};

auto formatKey(QString key) {
  auto asWords = QString{};
  auto prev = QChar{' '};
  for (auto c : key) {
    if (c.isUpper() || (c.isDigit() && prev.isDigit() == false)) {
      asWords += " ";
    }
    asWords += c;
    prev = c;
  }

  auto ret = QString{};
  auto lineLength = 0;
  for (auto word : asWords.split(" ")) {
    auto lineBreak = lineLength + word.length() > 21;
    if (lineBreak) {
      ret += "\n";
      lineLength = 0;
    }
    ret += word + " ";
    lineLength += word.length() + 1;
  }

  return ret;
}
}  // namespace

bool Debug::Populated() { return populated; }

struct Setting : public QWidget {
  const QString key;
  QLabel* value = new QLabel;
  QPushButton* resetButton = new QPushButton{"Reset"};

  Setting(QString key) : key(key) {
    auto layout = new QHBoxLayout;
    setLayout(layout);
    value->setText("foo");  // debug
    layout->addWidget(value);
    layout->addWidget(resetButton);
    // TODO here
    //  alignment of layout is bad, too much blank space
    // per item:
    //  value
    //  is default ?
    //  reset = set default (QToolButton)
    //  support for callback on change
    //  custom value with dial
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
    std::cout << "Setting: " << key.toStdString() << " = " << Settings::get(key, 0).toString().toStdString()
              << std::endl;

    auto name = formatKey(key);
    auto row = index / itemsPerRow;
    auto column = index % itemsPerRow;
    auto button = new QPushButton{name};
    layout->addWidget(button, row, column);
    buttons[key] = button;

    auto screenContents = new QWidget;  // todo
    auto screen = new SubScreen{name, screenContents};
    screens[key] = screen;

    ++index;
  }

  populated = true;
}

void Debug::connect(std::function<void()> goBackCallback, std::function<void(QWidget*)> goToSettingCallback) {
  for (auto [key, value] : buttons.asKeyValueRange()) {
    QObject::connect(value, &QAbstractButton::released, [&] { goToSettingCallback(screens[key]); });
  }
  for (auto screen : screens) {
    QObject::connect(screen->back, &QAbstractButton::released, [&] { goBackCallback(); });
  }
}
