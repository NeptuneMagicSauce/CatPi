#include "Emojis.hpp"

#include <stdexcept>

const char* Emojis::get(Emojis::Type type) {
  switch (type) {
    case Type::OkayWithThreeVSigns:
      return "✌️✌️✌️";
    case Type::WeightScale:
      return "⚖️";
  }
  throw std::runtime_error{"not implemented"};
}
