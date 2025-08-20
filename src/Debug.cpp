#include "Debug.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
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

  auto breakLines(const QString& line, auto maxLength) {
    auto ret = QString{};
    auto lineLength = 0;
    for (auto word : line.split(" ")) {
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
}

bool Debug::Populated() { return populated; }

struct Setting : public QWidget {
  const QString key;
  const QVariant defaultValue;
  const std::optional<int> minimum;
  const std::optional<int> maximum;
  const std::function<void(QVariant)> callback;
  QLabel* description = new QLabel;
  QLabel* value = new QLabel;
  QLabel* unit = new QLabel;
  QToolButton* resetButton = new QToolButton;
  DeltaDial* changeButton = new DeltaDial;
  QString previousValue;

  void updateValue() {
    auto newValue = Settings::get(key);
    value->setText(newValue.toString());
    resetButton->setEnabled(newValue != defaultValue);
  }

  Setting(const Settings::Load& load)
      : key(load.key),
        defaultValue(load.defaultValue),
        minimum(load.limits.minimum),
        maximum(load.limits.maximum),
        callback(load.callback) {
    Widget::FontSized(this, 20);

    for (auto widget : QList<QLabel*>{value, unit, description}) {
      Widget::AlignCentered(widget);
    }
    Widget::FontSized(value, 30);

    description->setText(breakLines(load.description, 45));
    updateValue();
    unit->setText(load.unit);
    resetButton->setText("Reset");
    resetButton->setIcon(QIcon{QPixmap{"://reset.png"}});
    resetButton->setIconSize({100, 100});

    auto valueWithUnit = new QWidget;
    auto valueLayout = new QVBoxLayout;
    valueWithUnit->setLayout(valueLayout);
    valueLayout->addWidget(value);
    if (load.unit.isEmpty() == false) {
      valueLayout->addWidget(unit);
    }

    auto bottom = new QGroupBox;
    auto bottomLayout = new QHBoxLayout;
    bottom->setLayout(bottomLayout);
    bottomLayout->addWidget(valueWithUnit);
    bottomLayout->addWidget(changeButton);
    bottomLayout->addWidget(resetButton);

    auto layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(description, 1);
    layout->addWidget(bottom, 2);
  }
};

Debug::Debug() {
  Widget::FontSized(this, 15);
  auto layout = new QGridLayout;
  setLayout(layout);
  auto const itemsPerRow = 3;
  int index = 0;

  QList<QString> obsoleteKeys;
  for (auto key : Settings::keys()) {
    if (Settings::isLoaded(key) == false) {
      obsoleteKeys.append(key);
      continue;
    }

    std::cout << "Setting: " << key.toStdString() << " = "
              << Settings::get(key).toString().toStdString() << std::endl;

    const auto& load = Settings::Load::get(key);

    auto setting = new Setting{load};
    auto button = new QPushButton{breakLines(load.name, 12)};
    auto screen = new SubScreen{load.name, setting};
    items[key] = {setting, button, screen};

    auto row = index / itemsPerRow;
    auto column = index % itemsPerRow;
    layout->addWidget(button, row, column);

    button->setMinimumHeight(80);

    ++index;
  }

  for (auto obsoleteKey : obsoleteKeys) {
    Settings::remove(obsoleteKey);
  }

  populated = true;
}

void Debug::connect(std::function<void()> goBackCallback,
                    std::function<void(QWidget*)> goToSettingCallback) {
  static auto itemChanged = [&](auto& item, auto newValue) {
    Settings::set(item.setting->key, newValue, false);
    item.setting->callback(newValue);
    item.setting->updateValue();
  };

  for (const auto& item : items) {
    QObject::connect(item.screen->back, &QAbstractButton::released, [&] { goBackCallback(); });
    QObject::connect(item.button, &QAbstractButton::released,
                     [&] { goToSettingCallback(item.screen); });
    QObject::connect(item.setting->resetButton, &QAbstractButton::released,
                     [&] { itemChanged(item, item.setting->defaultValue); });
    QObject::connect(item.setting->changeButton, &QAbstractSlider::valueChanged, [&] {
      auto newValue = Settings::get(item.setting->key).toInt() + item.setting->changeButton->delta;
      auto& minimum = item.setting->minimum;
      auto& maximum = item.setting->maximum;
      if ((minimum.has_value() && (newValue < minimum.value())) ||
          (maximum.has_value() && (newValue > maximum.value()))) {
        return;
      }
      itemChanged(item, newValue);
    });
  }
}

void Debug::changeFromOtherScreen(const QString& key) {
  if (Populated() == false) {
    // that's the initial load
    // we're not populated <-> items is empty
    return;
  }
  assert(items.contains(key));
  items[key].setting->updateValue();
}
