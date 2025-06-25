#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <map>

#include "Instance.hpp"
#include "Weight.hpp"

struct CentralWidgetImpl {
  CentralWidgetImpl(Instance*);
  QWidget* main = nullptr;
  QWidget* calibration = nullptr;
  QStackedLayout* layout = nullptr;
  QPushButton* dispense = nullptr;
  const std::map<CentralWidget::Page, int> indices = {{CentralWidget::Page::Main, 0},
                                                      {CentralWidget::Page::Calibration, 1}};
};

CentralWidget::CentralWidget(Instance* instance) : impl(new CentralWidgetImpl(instance)) {
  setLayout(impl->layout);
}

QPushButton* CentralWidget::widget() { return impl->dispense; }

CentralWidgetImpl::CentralWidgetImpl(Instance* instance) {
  main = new QWidget();
  auto layoutMain = new QHBoxLayout();
  main->setLayout(layoutMain);
  dispense = new QPushButton("Now!");
  dispense->setSizePolicy({dispense->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  layoutMain->addWidget(instance->weight->widget());
  layoutMain->addWidget(dispense);

  calibration = new QWidget();
  auto layoutCalibration = new QVBoxLayout();
  layoutCalibration->addWidget(new QLabel("foo"));
  calibration->setLayout(layoutCalibration);

  layout = new QStackedLayout();
  layout->insertWidget(indices.at(CentralWidget::Page::Main), main);
  layout->insertWidget(indices.at(CentralWidget::Page::Calibration), calibration);
}

void CentralWidget::setPage(Page page) { impl->layout->setCurrentIndex(impl->indices.at(page)); }
