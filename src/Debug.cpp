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

namespace {

auto populated = false;
auto buttons = QMap<QString, QPushButton*>{};
auto screens = QMap<QString, SubScreen*>{};

auto lineBreaked(const QString& line, auto maxLength) {
  auto asWords = QString{};
  auto prev = QChar{' '};
  for (auto c : line) {
    if (c.isUpper() || (c.isDigit() && prev.isDigit() == false)) {
      asWords += " ";
    }
    asWords += c;
    prev = c;
  }

  auto ret = QString{};
  auto lineLength = 0;
  for (auto word : asWords.split(" ")) {
    auto lineBreak = lineLength + word.length() > maxLength;
    if (lineBreak) {
      ret += "\n";
      lineLength = 0;
    }
    ret += word + " ";
    lineLength += word.length() + 1;
  }

  return ret.trimmed();
}
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
    // TODO here:
    //   pass init values to Settings::load() rather than get()
    //   support for callback on change
    // TODO DeltaDial: bigger maximum for smooth rotate, then divide delta
    description->setText(lineBreaked(
// TODO do not add blank spaces after upper case or digit here!
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore "
        "et dolore magna aliqua.",
        20));
    value->setText("1.23456");
    unit->setText("Milliseconds");
    isDefault->setText("Yes");
    resetButton->setText("Reset");

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
    std::cout << "Setting: " << key.toStdString() << " = " << Settings::get(key, 0).toString().toStdString()
              << std::endl;

    auto name = lineBreaked(key, 12);
    auto row = index / itemsPerRow;
    auto column = index % itemsPerRow;
    auto button = new QPushButton{name};
    button->setMinimumHeight(80);
    layout->addWidget(button, row, column);
    buttons[key] = button;

    auto screenContents = new Setting{key};
    auto screen = new SubScreen{name.replace("\n", ""), screenContents};
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
