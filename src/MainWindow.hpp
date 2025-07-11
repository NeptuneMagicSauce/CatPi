#include <QIcon>
#include <QMainWindow>
#include <QStyle>

struct QWidget;
struct QToolBar;

struct MainWindow : public QMainWindow {
  MainWindow(QWidget* centralWidget, QToolBar* toolbar);

  bool const isSmallScreen;

  void toggleFullscreen(bool& isFullscreen);
  void setAutomaticSize();

  void resizeEvent(QResizeEvent* event) override;

  static QIcon StandardIcon(QStyle::StandardPixmap);
};
