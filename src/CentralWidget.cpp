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
  QStackedLayout* pages = nullptr;
  QVBoxLayout* layout = nullptr;
  QLabel* statusMessage = nullptr;
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

  pages = new QStackedLayout;

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

void CentralWidget::setPage(QWidget* page) { impl->pages->setCurrentIndex(impl->subScreenIndices.at(page)); }

void CentralWidget::statusMessage(const QString& message) {
  impl->statusMessage->setText(message);
  impl->statusMessage->show();
  QTimer::singleShot(3000, [&] { impl->statusMessage->hide(); });
}
