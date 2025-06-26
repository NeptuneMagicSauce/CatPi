auto debugFormat = [](auto filename, auto widget) {
  auto file = QFile(filename);
  std::cout << filename << " " << file.exists() << std::endl;
  if (file.exists()) {
    file.open(QIODeviceBase::ReadOnly);
    try {
      widget->setStyleSheet(file.readAll());
    } catch (std::exception e) {
      std::cout << e.what() << std::endl;
    }
  }
  return widget;
};
