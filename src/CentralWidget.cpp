#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QTimer>
#include <map>

#include "System.hpp"

struct CentralWidgetImpl {
  CentralWidgetImpl(QWidget* weight, QWidget* calibration);
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

CentralWidget::CentralWidget(QWidget* weight, QWidget* calibration) {
  impl = new CentralWidgetImpl(weight, calibration);
  setLayout(impl->layout);
}

QAbstractButton* CentralWidget::dispenseButton() { return impl->dispense; }

CentralWidgetImpl::CentralWidgetImpl(QWidget* weight, QWidget* calibration) {
  AssertSingleton();
  auto main = new QWidget();
  auto layoutMain = new QHBoxLayout();
  main->setLayout(layoutMain);
  dispense = new QPushButton("Now!");
  dispense->setSizePolicy({dispense->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  layoutMain->addWidget(weight);
  layoutMain->addWidget(dispense);

  pages = new QStackedLayout();
  pages->insertWidget(indices.at(CentralWidget::Page::Main), main);
  pages->insertWidget(indices.at(CentralWidget::Page::Calibration), calibration);

  layout = new QVBoxLayout();
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
