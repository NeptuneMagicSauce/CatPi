#pragma once

struct QString;
struct QLabel;
struct QWidget;

struct Widget {
  static QWidget* AlignCentered(QLabel* widget);
  static QWidget* FontSized(QWidget* widget, int fontSize);
  static QWidget* Spacer();
};
