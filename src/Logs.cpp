#include "Logs.hpp"

#include <QBoxLayout>
#include <QTableWidget>
#include <iomanip>
#include <iostream>

#include "System.hpp"
#include "Widget.hpp"

namespace {
  QTableWidget* table = nullptr;

  auto stringToItem(const QString& str) {
    auto ret = new QTableWidgetItem(str);
    ret->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    return ret;
  }
}

Logs::Logs() {
  AssertSingleton();
  auto layout = new QVBoxLayout;
  setLayout(layout);

  table = new QTableWidget;
  layout->addWidget(table);

  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  table->setRowCount(8);
  table->setColumnCount(3);

  table->setHorizontalHeaderLabels({"Distribué", "Grammes", "Mangé"});

  table->setSelectionMode(QAbstractItemView::NoSelection);

  // size policy: not needed with spacer in class that composites me (Menu)
  // table->setSizePolicy({QSizePolicy::Policy::Minimum, table->sizePolicy().verticalPolicy()});

  Widget::FontSized(table, 15);

  setMinimumWidth(350);
}

void Logs::updateLogs(const QList<Event>& events) {
  // TODO measure cpu usage, only update on change
  if (events.isEmpty()) {
    return;
  }

  auto rowCount = table->rowCount();
  int row = 0;
  int eventIndex = events.size() - rowCount;
  if (eventIndex < 0) {
    eventIndex = 0;
  }
  while (eventIndex < events.size()) {
    auto const& event = events[eventIndex];

    // dispense
    table->setItem(row, 0, stringToItem(event.timeDispensed.time().toString()));

    // weight
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << event.grams;
    table->setItem(row, 1, stringToItem(QString::fromStdString(ss.str())));

    // eat
    if (event.timeEaten.has_value()) {
      table->setItem(row, 2, stringToItem(event.timeEaten.value().time().toString()));
    }

    ++row;
    ++eventIndex;
  }
}
