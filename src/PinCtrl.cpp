#include "PinCtrl.hpp"

#include <algorithm>
#include <ranges>
#include <vector>

extern "C" {
// that's the raspberry-utils program pinctrl as a lib
int main_pinctrl(int argc, char* argv[]);
}

using namespace std;

namespace PinCtrl {

void pinctrl(string_view sv) {
  // split on blank space
  auto splitted = sv | views::split(' ') | views::transform([](auto i) { return string{string_view{i}}; });
  // to vector<char*>
  vector<const char*> char_args = {"pinctrl"};
  ;
  ranges::transform(vector<string>{splitted.begin(), splitted.end()}, back_inserter(char_args),
                    [](auto& i) { return i.data(); });
  // // debug print
  // ranges::for_each(char_args, [](auto i) { cout << (void*)i << " " << i << endl; });
  main_pinctrl(char_args.size(), (char**)char_args.data());
}
}  
