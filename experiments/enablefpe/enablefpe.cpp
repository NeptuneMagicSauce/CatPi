// source https://stackoverflow.com/a/60731897

#include <fenv.h>

#include <cmath>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  cout << __PRETTY_FUNCTION__ << endl << endl;

  auto inf = [] {
    auto one = 1.0;  // when we inline the division, it does not raise an FPE with clang
    cout << "1.0 / 0.0 = " << endl;
    cout << one / 0.0 << endl;
  };
  auto nan = [] {
    cout << "-sqrt(-1.0) = " << endl;
    cout << -sqrt(-1.0) << endl;
  };

  inf();
  nan();

  cout << endl << "enable fpe exceptions" << endl << endl;
  feenableexcept(FE_DIVBYZERO | FE_INVALID);

  inf();
  nan();

  return 0;
}
