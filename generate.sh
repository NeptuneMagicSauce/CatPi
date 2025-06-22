set -e

sudo apt install -y \
        cmake \
        ninja-build \
        clang-19 \
        clang-format \
        qt6-base-dev \
        qt6-wayland \
        libfdt-dev

"$(dirname $0)/src/scripts/install-libpinctrl.sh"

rm -f CMakeCache.txt

cmake . \
      -G Ninja \
      --fresh \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=clang++-19 \
      -DLIBPINCTRLDIR=$HOME/raspberry-utils/pinctrl
