set -e

sudo apt install -y \
        cmake \
        ninja-build \
        clang-19 \
        clangd-19 \
        clang-format-19 \
        binutils \
        libxkbcommon-dev \
        qt6-base-dev \
        qt6-charts-dev \
        qt6-wayland

for i in clang clangd clang-format
do
    sudo update-alternatives --install /usr/bin/$i $i /usr/bin/$i-19 100
done

"$(dirname $0)/src/scripts/install-libpinctrl.sh"
"$(dirname $0)/src/scripts/install-libhx711.sh"

rm -fr CMakeCache.txt build.ninja *_autogen/ CMakeFiles/ cmake_install.cmake  compile_commands.json

cmake . \
      -G Ninja \
      --fresh \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=clang++-19 \
      -DLIBPINCTRLDIR=$HOME/raspberry-utils/pinctrl \
      -DLIBHX711=$HOME/libhx711
