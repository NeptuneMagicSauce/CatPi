#include "Logs.hpp"

#include <QBoxLayout>
#include <QTableWidget>

#include "System.hpp"
#include "Widget.hpp"

namespace {
  QTableWidget* table = nullptr;

  auto dateTimeToItem(const QDateTime& dateTime) {
    auto ret = new QTableWidgetItem(dateTime.time().toString());
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
  table->setColumnCount(2);

  table->setHorizontalHeaderLabels({"Distribué", "Mangé"});

  table->setSelectionMode(QAbstractItemView::NoSelection);

  // size policy: not needed with spacer in class that composites me (Menu)
  // table->setSizePolicy({QSizePolicy::Policy::Minimum, table->sizePolicy().verticalPolicy()});

  Widget::FontSized(table, 15);
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
    table->setItem(row, 0, dateTimeToItem(event.timeDispensed));
    if (event.timeEaten.has_value()) {
      table->setItem(row, 1, dateTimeToItem(event.timeEaten.value()));
    }
    ++row;
    ++eventIndex;
  }
}
