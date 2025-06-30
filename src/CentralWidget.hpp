#pragma once

#include <QList>
#include <QWidget>

struct QAbstractButton;
struct QWidget;
struct SubScreen;

struct CentralWidget : public QWidget {
  CentralWidget(QWidget* weight, QWidget* delay, QList<SubScreen*> subScreens);

  QAbstractButton* dispenseButton();

  void setPage(QWidget* page);

  void statusMessage(const QString&);
};
