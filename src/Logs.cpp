#include "Logs.hpp"

#include <QApplication>
#include <QBoxLayout>
#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QFile>
#include <QLineSeries>
#include <QMainWindow>
#include <QShortcut>
#include <QStandardPaths>
#include <QValueAxis>
#include <iostream>

namespace {
  void foobar() {
    qDebug() << Q_FUNC_INFO;
    auto f = QFile{
        QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation).first() +
        "/logs.txt"};
    f.open(QIODeviceBase::ReadOnly);
    QList<double> listRaw;
    for (auto& line : f.readAll().split('\n')) {
      auto items = line.split(',');
      if (items.size() < 2) {
        continue;
      }
      if (items.first().contains("Dispense")) {
        continue;
      }
      auto weight = items[1].toDouble();
      listRaw << weight;

      if (listRaw.size() == 10000 / 2) {
        break;
      }
    }

    auto raw = new QLineSeries;
    raw->setName("Raw");

    auto insertAverage = [&](auto index, auto averageRange, auto& result) {
      if (index == 0) {
        result->setName("Average " + QString::number(averageRange));
      }
      auto count = 0;
      auto total = 0.0;
      for (int i = index - averageRange; i <= index; ++i) {
        if (i < 0) {
          continue;
        }
        total += listRaw[i];
        ++count;
      }
      result->append(index, total / count);
    };
    auto insertMedian = [&](auto index, auto medianRange, auto& result) {
      if (index == 0) {
        result->setName("Median " + QString::number(medianRange));
      }
      medianRange += 1 - (medianRange % 2);  // this algo works on odd counts

      // https://stackoverflow.com/a/1719155
      std::vector<double> v;
      v.reserve(medianRange + 1);
      for (int i = (index - medianRange) + 1; i <= index; ++i) {
        if (i < 0) {
          continue;
        }
        v.emplace_back(listRaw[i]);
        // std::cout << v[v.size() - 1] << ",";
      }

      auto n = v.size() / 2;
      nth_element(v.begin(), v.begin() + n, v.end());
      result->append(index, v[n]);
      // std::cout << "=," << v[n] << std::endl;
    };

    auto average10 = new QLineSeries;
    auto average50 = new QLineSeries;
    auto average100 = new QLineSeries;
    auto median10 = new QLineSeries;
    auto median20 = new QLineSeries;  // best one
    auto median30 = new QLineSeries;
    auto median50 = new QLineSeries;
    auto median100 = new QLineSeries;
    for (int index = 0; index < listRaw.size(); ++index) {
      raw->append(index, listRaw[index]);
      insertAverage(index, 10, average10);
      insertAverage(index, 50, average50);
      insertAverage(index, 100, average100);
      insertMedian(index, 10, median10);
      insertMedian(index, 20, median20);
      insertMedian(index, 30, median30);
      insertMedian(index, 50, median50);
      insertMedian(index, 100, median100);
    }
    auto chart = new QChart;
    auto yaxis = new QValueAxis;
    yaxis->setRange(-3, 10);
    chart->addAxis(yaxis, Qt::AlignLeft);

    for (auto series : {
             raw,  //
             // average10,  //
             // average50,   //
             // average100,  //
             median10,  //
             median20,
             // median30,  //
             // median50,  //
             // median100    //
         }) {
      chart->addSeries(series);
      series->attachAxis(yaxis);
    }

    auto dialog = new QDialog;
    auto layout = new QVBoxLayout;
    dialog->setLayout(layout);
    layout->addWidget(new QChartView(chart));
    dialog->showMaximized();
    dialog->exec();
  }
}

void Logs::connect(QMainWindow* window) {
  auto shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F2), window);
  shortcut->setContext(Qt::ApplicationShortcut);
  QObject::connect(shortcut, &QShortcut::activated, qApp, [&]() { foobar(); });
}
