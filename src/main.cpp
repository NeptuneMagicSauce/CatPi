#include <QApplication>
#include "Instance.hpp"

int main(int argc, char **argv) {
  return Instance{argc, argv}.app->exec();
}
