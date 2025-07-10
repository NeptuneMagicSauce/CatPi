struct QString;
struct QAbstractButton;
#include <QWidget>

struct SubScreen : public QWidget {
  SubScreen(const QString& title, QWidget* contents);

  QAbstractButton* back = nullptr;
  const QWidget* contents = nullptr;

  static void connect(std::function<void()> callback);
};
