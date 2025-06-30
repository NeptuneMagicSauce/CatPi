#include "Widget.hpp"

#include <QLabel>
#include <QString>
#include <QWidget>

namespace {
QWidget* StyleSheeted(QWidget* widget, const QString& styleSheet) {
  widget->setStyleSheet(styleSheet);
  return widget;
}
}  // namespace

QWidget* Widget::FontSized(QWidget* widget, int fontSize) {
  return StyleSheeted(widget, QString("QWidget{font-size: ") + QString::number(fontSize) + QString("pt; }"));
}

QWidget* Widget::AlignCentered(QLabel* widget) {
  widget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  return widget;
}
