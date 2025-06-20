# liblgpio-dev
sudo apt install -y \
        cmake \
        ninja-build \
        clang-19 \
        qt6-base-dev \
        qt6-wayland \
        libfdt-dev \


rm -f CMakeCache.txt

cmake . \
      -G Ninja \
      --fresh \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=clang++-19 \
      -DLIBPINCTRLDIR=$HOME/raspberry-utils/pinctrl
