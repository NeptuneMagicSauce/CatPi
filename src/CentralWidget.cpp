#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QTimer>
#include <map>

#include "SubScreen.hpp"
#include "System.hpp"

struct CentralWidgetImpl {
  CentralWidgetImpl(QWidget* weight, QWidget* delay, QList<SubScreen*> subScreens);
  QStackedLayout* pages = nullptr;
  QPushButton* dispense = nullptr;
  QVBoxLayout* layout = nullptr;
  QLabel* statusMessage = nullptr;
  std::map<const QWidget*, int> subScreenIndices;
};

namespace {
CentralWidgetImpl* impl = nullptr;
}

CentralWidget::CentralWidget(QWidget* weight, QWidget* delay, QList<SubScreen*> subScreens) {
  impl = new CentralWidgetImpl(weight, delay, subScreens);
  setLayout(impl->layout);
}

QAbstractButton* CentralWidget::dispenseButton() { return impl->dispense; }

CentralWidgetImpl::CentralWidgetImpl(QWidget* weight, QWidget* delay, QList<SubScreen*> subScreens) {
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
  pages->addWidget(main);

  for (auto subScreen : subScreens) {
    subScreenIndices[subScreen->contents] = pages->count();
    pages->addWidget(subScreen);
  }

  layout = new QVBoxLayout;
  statusMessage = new QLabel;
  statusMessage->setStyleSheet("QWidget{font-size: 25pt;}");
  statusMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addLayout(pages);
  layout->addWidget(statusMessage);
  statusMessage->hide();
}

void CentralWidget::setPage(QWidget* page) {
  auto index = (page == nullptr) ? 0 : impl->subScreenIndices.at(page);
  impl->pages->setCurrentIndex(index);
}

void CentralWidget::statusMessage(const QString& message) {
  impl->statusMessage->setText(message);
  impl->statusMessage->show();
  QTimer::singleShot(3000, [&] { impl->statusMessage->hide(); });
}
