#include <QList>
#include <QWidget>

struct QAbstractButton;
struct SubScreen;

struct CentralWidget : public QWidget {
  CentralWidget(QList<SubScreen*> subScreens);

  void setPage(QWidget* page);
  void setSettingPage(QWidget* page);

  void statusMessage(const QString&);
};
