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
#include <ostream>

#include "Logs.hpp"
#include "System.hpp"
#include "Widget.hpp"

using std::endl;

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

  struct TabulatedData {
    QMap<int, double> eatenWeightsPerHour;
    double maxPerHour = 0;
    double totalPeriod = 0;
  };
  TabulatedData tabulate(const QDate& day);
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
  barSeries.setBarWidth(1);  // 100% of the cell width
  barSeries.append(&bars);

  xAxis.setRange(0, 24);
  xAxis.setTickCount(9);
  xAxis.setGridLineVisible(false);

  xAxis.setLabelFormat("%dH");
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
void LogsWidget::showEvent(QShowEvent* event) {
  // reset displayed day to current when we are shown
  impl->current = QDate::currentDate();
  loadData();
  QWidget::showEvent(event);
}

LogsWidgetImpl::TabulatedData LogsWidgetImpl::tabulate(const QDate& day) {
  auto ret = TabulatedData{};

  QMap<int, QList<Logs::Event>> eventsPerHour;
  for (const auto& d : logs.readHistoricalData(day)) {
    if (d.type == Logs::Event::Type::Eat) {
      eventsPerHour[d.time.time().hour()].append(d);
    }
  }
  for (int hour = 0; hour < 24; ++hour) {
    auto totalPerHour = double{0};
    if (eventsPerHour.contains(hour)) {
      for (const auto& e : eventsPerHour[hour]) {
        // qDebug() << "EAT" << e.time.toString() << e.weight;
        totalPerHour += e.weight;
      }
    }
    ret.maxPerHour = std::max(ret.maxPerHour, totalPerHour);
    ret.totalPeriod += totalPerHour;
    ret.eatenWeightsPerHour[hour] = totalPerHour;
  }

  return ret;
}

void LogsWidget::loadData() { impl->loadData(); }

void LogsWidgetImpl::loadData() {
  title->setText(FormatDay(current));
  for (auto timeDirection : timeButtons.keys()) {
    auto const& timeButton = timeButtons[timeDirection];
    auto enabled = logs.hasHistoricalData(current.addDays(timeDirection));
    timeButton.button->setEnabled(enabled);
    timeButton.button->setIcon(enabled ? timeButton.icon : QIcon{});
  }

  bars.remove(0, bars.count());
  yAxis.setTitleText("Grammes");

  const auto tabulatedData = tabulate(current);

  for (int hour = 0; hour < 24; ++hour) {
    // qDebug() << "hour" << hour << "weight" << eatenWeightsPerHour[hour];
    bars << tabulatedData.eatenWeightsPerHour[hour];
  }

  // it fails when we add the series to the chart earlier in the constructor !?
  if (chart.series().empty()) {
    chart.addSeries(&barSeries);
    barSeries.attachAxis(&yAxis);
    // barSeries.attachAxis(&yAxis);
  }
  auto constexpr subTickPerTick = 5;
  auto maxYValue = (int)tabulatedData.maxPerHour + 1;
  maxYValue += subTickPerTick - (maxYValue % subTickPerTick);
  maxYValue = std::max(maxYValue, subTickPerTick * 3);  // minimum 3 ticks
  yAxis.setMax(maxYValue);
  yAxis.setTickCount((maxYValue / subTickPerTick) + 1);
  yAxis.setMinorTickCount(subTickPerTick - 1);

  bars.setLabel("Grammes / Heure, Total Jour = " + QString::number((int)tabulatedData.totalPeriod));
}

QString LogsWidget::asAscii(const QDate& date) {
  auto formatIntegerTwoDigits = [](auto& cout, int value) {
    if (value < 10) {
      cout << "0";
    }
    cout << value;
  };

  auto cout = std::ostringstream{};
  const auto tabulatedData = impl->tabulate(date);

  cout << "Total: " << (int)tabulatedData.totalPeriod << " grammes" << endl;

  static auto cornerTopLeft = "┌";
  static auto cornerTopRight = "┐";
  static auto cornerBottomLeft = "└";
  static auto cornerBottomRight = "┘";
  static auto barVertical = "│";
  static auto barHorizontal = "─";
  static auto emptyChar = " ";  // U+2007 FIGURE SPACE, this works as fixed width on gmail-ios
  static auto emptyTimePrefix = std::string{emptyChar} + std::string{emptyChar};

  cout << emptyTimePrefix << cornerTopLeft;
  for (int hour = 0; hour < 24; ++hour) {
    cout << barHorizontal;
  }
  cout << cornerTopRight << endl;

  static auto const maxValue = 12 - 1;
  for (int height = maxValue; height >= 0; --height) {
    auto actualHeight = height + 1;
    if (actualHeight % 4 == 0) {
      // cout << std::format("{:02}", height); // not available on pi
      formatIntegerTwoDigits(cout, actualHeight);
    } else {
      cout << emptyTimePrefix;
    }
    cout << barVertical;
    for (int hour = 0; hour <= 24; ++hour) {
      if (hour == 24) {
        cout << barVertical;
        continue;
      }
      auto value = tabulatedData.eatenWeightsPerHour[hour];
      if (value > height) {
        cout << "█";
      } else {
        cout << emptyChar;
      }
    }
    cout << endl;
  }

  cout << "00" << cornerBottomLeft;
  for (int hour = 0; hour < 24; ++hour) {
    if (tabulatedData.eatenWeightsPerHour[hour] > 0) {
      cout << "▀";
    } else {
      cout << barHorizontal;
    }
  }
  cout << cornerBottomRight << endl;

  cout << emptyTimePrefix;
  for (int hour = 0; hour <= 24; ++hour) {
    if (hour % 3 == 0) {
      formatIntegerTwoDigits(cout, hour);
    } else if (hour % 3 == 2) {
      cout << " ";
    }
  }
  cout << endl;

  return QString::fromStdString(cout.str());
}
