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
  CentralWidgetImpl(QList<SubScreen*> subScreens);
  QStackedLayout* pages = new QStackedLayout;
  QVBoxLayout* layout = new QVBoxLayout;
  QLabel* statusMessage = new QLabel;
  std::map<const QWidget*, int> subScreenIndices;
};

namespace {
CentralWidgetImpl* impl = nullptr;
}

CentralWidget::CentralWidget(QList<SubScreen*> subScreens) {
  impl = new CentralWidgetImpl(subScreens);
  setLayout(impl->layout);
}

CentralWidgetImpl::CentralWidgetImpl(QList<SubScreen*> subScreens) {
  AssertSingleton();

  for (auto subScreen : subScreens) {
    subScreenIndices[subScreen->contents] = pages->count();
    pages->addWidget(subScreen);
  }

  statusMessage->setStyleSheet("QWidget{font-size: 25pt;}");
  statusMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addLayout(pages);
  layout->addWidget(statusMessage);
  statusMessage->hide();
}

void CentralWidget::setPage(QWidget* page) {
  assert(impl->subScreenIndices.at(page) < impl->pages->count());
  impl->pages->setCurrentIndex(impl->subScreenIndices.at(page));
}

void CentralWidget::setSettingPage(QWidget* page) {
  if (impl->subScreenIndices.contains(page) == false) {
    impl->subScreenIndices[page] = impl->pages->count();
    impl->pages->addWidget(page);
  }
  setPage(page);
}

void CentralWidget::statusMessage(const QString& message) {
  impl->statusMessage->setText(message);
  impl->statusMessage->show();
  QTimer::singleShot(3000, [&] { impl->statusMessage->hide(); });
}
