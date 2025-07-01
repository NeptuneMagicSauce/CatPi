#include "Debug.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <iostream>

#include "Settings.hpp"

namespace {
auto populated = false;
}

bool Debug::Populated() { return populated; }

struct Setting : public QWidget {
  QLabel* name = new QLabel;
  QLabel* value = new QLabel;
  QPushButton* resetButton = new QPushButton{"Reset"};
  QList<QWidget*> widgets = {name, value, resetButton};

  Setting(QString setting) {
    name->setText(setting);
    name->setSizePolicy(QSizePolicy::Expanding, name->sizePolicy().verticalPolicy());

    value->setText("foo");
    // TODO here
    //  alignment of layout is bad, too much blank space
    // per item:
    //  value
    //  is default ?
    //  reset = set default (QToolButton)
    //  support for callback on change
    //  custom value with dial

    // value->setText(Settings::get(key)).toString();
  }
};

struct DebugWidget : public QWidget {
  DebugWidget() {
    setStyleSheet("QWidget{font-size: 15pt;}");
    auto layout = new QGridLayout;
    setLayout(layout);
    int row = 0;
    for (auto key : Settings::keys()) {
      std::cout << "Setting: " << key.toStdString() << " = " << Settings::get(key, 0).toString().toStdString()
                << std::endl;
      auto setting = new Setting{key};
      auto column = 0;
      for (auto widget : setting->widgets) {
        // debug to remove :
        // auto alignment = column == 0 ? Qt::AlignLeft : Qt::AlignRight;
        layout->addWidget(widget, row, column++);
      }
      ++row;
    }
    populated = true;
  }
};

Debug::Debug() { setWidget(new DebugWidget); }
