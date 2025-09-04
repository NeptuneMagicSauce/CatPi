#include "LogsWidget.hpp"

#include <QBarSeries>
#include <QBarSet>
#include <QBoxLayout>
#include <QChartView>
#include <QDateTime>
#include <QGroupBox>
#include <QLabel>
#include <QValueAxis>

#include "Logs.hpp"
#include "System.hpp"
#include "Widget.hpp"

struct LogsWidgetImpl {
  LogsWidgetImpl(Logs const& logs);

  Logs const& logs;

  QLayout* layout = nullptr;
  QLabel* title = nullptr;
  QDate current = QDate::currentDate();

  QBarSeries barSeries;
  QBarSet bars{""};

  QChart chart;
  QValueAxis xAxis, yAxis;
  QChartView chartView;

  static QString FormatDay(const auto& day);
  void loadData();
};

namespace {
  LogsWidgetImpl* impl = nullptr;
}

QString LogsWidgetImpl::FormatDay(const auto& day) {
  static auto const namesOfDays = QMap<int, QString>{
      {1, "Lundi"},    {2, "Mardi"},  {3, "Mercredi"}, {4, "Jeudi"},
      {5, "Vendredi"}, {6, "Samedi"}, {7, "Dimanche"},
  };
  static auto const namesOfMonths = QMap<int, QString>{
      {1, "Janvier"},   {2, "Février"},  {3, "Mars"},      {4, "Avril"},
      {5, "Mai"},       {6, "Juin"},     {7, "Juillet"},   {8, "Août"},
      {9, "Septembre"}, {10, "Octobre"}, {11, "Novembre"}, {12, "Décembre"},
  };

  auto ret = QString{};
  ret += namesOfDays[day.dayOfWeek()];
  ret += " ";
  ret += QString::number(day.day());
  ret += " ";
  ret += namesOfMonths[day.month()];
  ret += " ";
  ret += QString::number(day.year());
  return ret;
}

LogsWidget::LogsWidget(Logs const& logs) {
  impl = new LogsWidgetImpl{logs};
  setLayout(impl->layout);
}

LogsWidgetImpl::LogsWidgetImpl(Logs const& logs) : logs(logs) {
  layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);  // we will need all the space we can get

  auto titleParent = new QGroupBox;
  auto titleLayout = new QVBoxLayout;
  titleParent->setLayout(titleLayout);
  titleParent->setMaximumHeight(50);
  title = new QLabel;
  titleLayout->addWidget(title);

  Widget::FontSized(title, 15);
  Widget::AlignCentered(title);

  chart.setMargins({0, 0, 0, 0});

  chartView.setChart(&chart);
  barSeries.append(&bars);

  yAxis.setRange(0, 24);
  yAxis.setTickCount(25);
  yAxis.setLabelFormat("%d");
  yAxis.setTitleText("Heures");
  chart.addAxis(&yAxis, Qt::AlignBottom);

  xAxis.setLabelFormat("%d");
  chart.addAxis(&xAxis, Qt::AlignLeft);

  layout->addWidget(titleParent);
  layout->addWidget(&chartView);
}

void LogsWidget::loadData() { impl->loadData(); }

void LogsWidgetImpl::loadData() {
  title->setText(FormatDay(current));
  auto const& data = logs.readHistoricalData(current);

  bars.remove(0, bars.count());

  QMap<int, QList<Logs::Event>> eventsPerHour;
  QMap<int, double> eatenWeightsPerHour;
  bars.setLabel("Grammes mangés par heure");
  xAxis.setTitleText("Grammes");
  for (const auto& d : data) {
    if (d.type == Logs::Event::Type::Eat) {
      eventsPerHour[d.time.time().hour()].append(d);
    }
  }
  auto maxPerHour = double{0};
  for (int hour = 0; hour < 24; ++hour) {
    auto total = double{0};
    if (eventsPerHour.contains(hour)) {
      for (const auto& e : eventsPerHour[hour]) {
        // qDebug() << "EAT" << e.time.toString() << e.weight;
        total += e.weight;
      }
    }
    maxPerHour = std::max(maxPerHour, total);
    eatenWeightsPerHour[hour] = total;
  }
  for (int hour = 0; hour < 24; ++hour) {
    // qDebug() << "hour" << hour << "weight" << eatenWeightsPerHour[hour];
    bars << eatenWeightsPerHour[hour];
  }

  // it fails when we add the series to the chart earlier in the constructor !?
  if (chart.series().empty()) {
    chart.addSeries(&barSeries);
    barSeries.attachAxis(&xAxis);
    // barSeries.attachAxis(&yAxis);
  }
  auto maxXValue = (int)maxPerHour + 1;
  xAxis.setMax(maxXValue);
  xAxis.setTickCount(maxXValue + 1);

  // TODO here
  // include total of the view
  // day navigation +/- with buttons around title
  // other set: events eaten
  // remember the settings: selected set, selected time scale
  // other time scales: last 7 days, last 30 days, last year
}
