#include "Widget.hpp"

#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QString>
#include <QWidget>

QString Widget::StyleSheetFontSize(int fontSize) {
  return QString{"QWidget{font-size: "} + QString::number(fontSize) + QString{"pt; }"};
}

QWidget* Widget::FontSized(QWidget* widget, int fontSize) {
  widget->setStyleSheet(StyleSheetFontSize(fontSize));
  return widget;
}

QWidget* Widget::AlignCentered(QLabel* widget) {
  widget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  return widget;
}

QWidget* Widget::Spacer() {
  auto spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  return spacer;
}

bool Widget::IsSmallScreen() {
  return QGuiApplication::primaryScreen()->geometry().height() <= 720;
}
