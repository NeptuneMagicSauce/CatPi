#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <map>

#include "System.hpp"

struct CentralWidgetImpl {
  CentralWidgetImpl(QWidget* weight, QWidget* calibration);
  QWidget* main = nullptr;
  QWidget* calibration = nullptr;
  QStackedLayout* layout = nullptr;
  QPushButton* dispense = nullptr;
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
  main = new QWidget();
  auto layoutMain = new QHBoxLayout();
  main->setLayout(layoutMain);
  dispense = new QPushButton("Now!");
  dispense->setSizePolicy({dispense->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  layoutMain->addWidget(weight);
  layoutMain->addWidget(dispense);

  layout = new QStackedLayout();
  layout->insertWidget(indices.at(CentralWidget::Page::Main), main);
  layout->insertWidget(indices.at(CentralWidget::Page::Calibration), calibration);
}

void CentralWidget::setPage(Page page) { impl->layout->setCurrentIndex(impl->indices.at(page)); }
