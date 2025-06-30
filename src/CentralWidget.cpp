#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QTimer>
#include <map>

#include "System.hpp"

struct CentralWidgetImpl {
  CentralWidgetImpl(QWidget* weight, QWidget* calibration, QWidget* delay);
  QStackedLayout* pages = nullptr;
  QPushButton* dispense = nullptr;
  QVBoxLayout* layout = nullptr;
  QLabel* statusMessage = nullptr;
  const std::map<CentralWidget::Page, int> indices = {{CentralWidget::Page::Main, 0},
                                                      {CentralWidget::Page::Calibration, 1}};
};

namespace {
CentralWidgetImpl* impl = nullptr;
}

CentralWidget::CentralWidget(QWidget* weight, QWidget* calibration, QWidget* delay) {
  impl = new CentralWidgetImpl(weight, calibration, delay);
  setLayout(impl->layout);
}

QAbstractButton* CentralWidget::dispenseButton() { return impl->dispense; }

CentralWidgetImpl::CentralWidgetImpl(QWidget* weight, QWidget* calibration, QWidget* delay) {
  AssertSingleton();
  auto main = new QWidget;

  auto layoutMainTop = new QHBoxLayout;
  dispense = new QPushButton("Now!");
  dispense->setSizePolicy({dispense->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  layoutMainTop->addWidget(weight);
  layoutMainTop->addWidget(dispense);

  auto layoutMain = new QVBoxLayout;
  main->setLayout(layoutMain);
  layoutMain->addLayout(layoutMainTop, 2);
  layoutMain->addWidget(delay, 1);

  pages = new QStackedLayout;
  pages->insertWidget(indices.at(CentralWidget::Page::Main), main);
  pages->insertWidget(indices.at(CentralWidget::Page::Calibration), calibration);

  layout = new QVBoxLayout;
  statusMessage = new QLabel;
  statusMessage->setStyleSheet("QWidget{font-size: 25pt;}");
  statusMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addLayout(pages);
  layout->addWidget(statusMessage);
  statusMessage->hide();
}

void CentralWidget::setPage(Page page) { impl->pages->setCurrentIndex(impl->indices.at(page)); }

void CentralWidget::statusMessage(const QString& message) {
  impl->statusMessage->setText(message);
  impl->statusMessage->show();
  QTimer::singleShot(3000, [&] { impl->statusMessage->hide(); });
}
