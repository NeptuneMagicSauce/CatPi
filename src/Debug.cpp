#include "Debug.hpp"

#include <iostream>

#include "Settings.hpp"

namespace {
auto populated = false;
}

bool Debug::Populated() { return populated; }

Debug::Debug() {
  for (auto key : Settings::keys()) {
    std::cout << "Setting: " << key.toStdString() << " = " << Settings::get(key, 0).toString().toStdString()
              << std::endl;
    // TODO here
    // scroll area
    // list-widget: sheet?
    // widget per setting ->
    //  name
    //  value
    //  is default ?
    //  support for callback on change
    //  set default
    //  custom value with dial
  }
  populated = true;
}
