#include "LogsWidget.hpp"

#include <QBarSeries>
#include <QBarSet>
#include <QBoxLayout>
#include <QChartView>
#include <QDateTime>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QValueAxis>

#include "Logs.hpp"
#include "System.hpp"
#include "Widget.hpp"

struct LogsWidgetImpl {
  LogsWidgetImpl(Logs const& logs);

  Logs const& logs;

  QDate current = QDate::currentDate();

  QLayout* layout = nullptr;
  QLabel* title = nullptr;

  QAbstractButton* buttonNext = nullptr;
  QAbstractButton* buttonPrevious = nullptr;
  struct TimeButton {
    QAbstractButton* button;
    QIcon icon;
  };
  QMap<int, TimeButton> timeButtons;

  QBarSeries barSeries;
  QBarSet bars{""};

  QChart chart;
  QValueAxis yAxis, xAxis;
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
  auto titleLayout = new QHBoxLayout;
  titleParent->setLayout(titleLayout);
  titleParent->setMaximumHeight(50);
  title = new QLabel;
  titleLayout->setContentsMargins(0, 0, 0, 0);

  auto titleButton = [this](const QString& iconName, int changeDirection, bool rotateIcon) {
    auto ret = new QToolButton;
    auto icon = QIcon{QPixmap{"://" + iconName}};
    auto size = QSize{150, 50};
    if (rotateIcon) {
      icon = Widget::RotateIcon(icon, 180, size);
    }
    ret->setIcon(icon);
    ret->setIconSize(size);
    QObject::connect(ret, &QAbstractButton::released, [this, changeDirection] {
      current = current.addDays(changeDirection);
      loadData();
    });
    timeButtons[changeDirection] = {.button = ret, .icon = icon};
    return ret;
  };

  titleLayout->addWidget(Widget::Spacer());
  titleLayout->addWidget(buttonPrevious = titleButton("back.png", -1, false));
  titleLayout->addWidget(Widget::Spacer());
  titleLayout->addWidget(title);
  titleLayout->addWidget(Widget::Spacer());
  titleLayout->addWidget(buttonNext = titleButton("back.png", +1, true));
  titleLayout->addWidget(Widget::Spacer());

  Widget::FontSized(title, 15);
  Widget::AlignCentered(title);

  chart.setMargins({0, 0, 0, 0});

  chartView.setChart(&chart);
  barSeries.append(&bars);

  xAxis.setRange(0, 24);
  xAxis.setTickCount(25);
  xAxis.setGridLineVisible(false);

  xAxis.setLabelFormat("%d");
  xAxis.setTitleText("Heures");
  chart.addAxis(&xAxis, Qt::AlignBottom);

  yAxis.setLabelFormat("%d");
  auto gridLinePen = QPen{};
  gridLinePen.setWidth(2);
  gridLinePen.setColor(QColor{192, 192, 192});
  yAxis.setGridLinePen(gridLinePen);
  chart.addAxis(&yAxis, Qt::AlignLeft);

  layout->addWidget(titleParent);
  layout->addWidget(&chartView);
}

void LogsWidget::loadData() { impl->loadData(); }

void LogsWidgetImpl::loadData() {
  title->setText(FormatDay(current));
  auto const& data = logs.readHistoricalData(current);

  for (auto timeDirection : timeButtons.keys()) {
    auto const& timeButton = timeButtons[timeDirection];
    auto enabled = logs.hasHistoricalData(current.addDays(timeDirection));
    timeButton.button->setEnabled(enabled);
    timeButton.button->setIcon(enabled ? timeButton.icon : QIcon{});
  }

  bars.remove(0, bars.count());

  QMap<int, QList<Logs::Event>> eventsPerHour;
  QMap<int, double> eatenWeightsPerHour;
  yAxis.setTitleText("Grammes");
  for (const auto& d : data) {
    if (d.type == Logs::Event::Type::Eat) {
      eventsPerHour[d.time.time().hour()].append(d);
    }
  }
  auto maxPerHour = double{0};
  auto totalPeriod = double{0};
  for (int hour = 0; hour < 24; ++hour) {
    auto totalPerHour = double{0};
    if (eventsPerHour.contains(hour)) {
      for (const auto& e : eventsPerHour[hour]) {
        // qDebug() << "EAT" << e.time.toString() << e.weight;
        totalPerHour += e.weight;
      }
    }
    maxPerHour = std::max(maxPerHour, totalPerHour);
    totalPeriod += totalPerHour;
    eatenWeightsPerHour[hour] = totalPerHour;
  }
  for (int hour = 0; hour < 24; ++hour) {
    // qDebug() << "hour" << hour << "weight" << eatenWeightsPerHour[hour];
    bars << eatenWeightsPerHour[hour];
  }

  // it fails when we add the series to the chart earlier in the constructor !?
  if (chart.series().empty()) {
    chart.addSeries(&barSeries);
    barSeries.attachAxis(&yAxis);
    // barSeries.attachAxis(&yAxis);
  }
  auto maxXValue = (int)maxPerHour + 1;
  auto constexpr subTickPerTick = 5;
  maxXValue += subTickPerTick - (maxXValue % subTickPerTick);
  yAxis.setMax(maxXValue);
  yAxis.setTickCount((maxXValue / subTickPerTick) + 1);
  yAxis.setMinorTickCount(subTickPerTick - 1);

  // no markers between hours
  // hours label offset 1/2 unit right
  // hide 24.5!
  // bars wider

  bars.setLabel("Grammes / Heure, Total Jour = " + QString::number((int)totalPeriod));
}
