#include "LogsWidget.hpp"

#include <QBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <iomanip>
#include <iostream>

#include "System.hpp"
#include "Widget.hpp"

namespace {
  QTableWidget* table = nullptr;
  QLabel* summary;

  auto stringToItem(const QString& str) {
    auto ret = new QTableWidgetItem(str);
    ret->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    return ret;
  }

  auto formatWeight(const auto& grams) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << grams;
    return QString::fromStdString(ss.str());
  }
}

LogsWidget::LogsWidget() {
  AssertSingleton();

  auto layout = new QVBoxLayout;
  setLayout(layout);

  Widget::FontSized(this, 14);

  table = new QTableWidget;
  layout->addWidget(table);

  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  table->setRowCount(6);
  table->setColumnCount(3);

  table->setHorizontalHeaderLabels({"Distribué", "Grammes", "Mangé"});

  table->setSelectionMode(QAbstractItemView::NoSelection);

  setMinimumWidth(335);

  summary = new QLabel;
  layout->addWidget(summary);
}

void LogsWidget::update(const QList<Event>& events) {
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
    table->setItem(row, 1, stringToItem(formatWeight(event.grams)));

    // eat
    table->setItem(
        row, 2,
        stringToItem(event.timeEaten.has_value() ? event.timeEaten.value().time().toString() : ""));

    ++row;
    ++eventIndex;
  }

  auto now = QDateTime::currentDateTime();
  auto dateNow = now.date();
  auto totalToday = 0.0;
  auto total24hours = 0.0;
  for (auto const& event : events) {
    if (event.timeDispensed.date() == dateNow) {
      totalToday += event.grams;
    }
    if (event.timeDispensed.secsTo(now) < 60 * 60 * 24) {
      total24hours += event.grams;
    }
  }

  summary->setText("Total Jour: " + formatWeight(totalToday) + " grammes\n" +
                   "Total 24 heures: " + formatWeight(total24hours) + " grammes");
}
