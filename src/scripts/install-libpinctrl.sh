set -e

sudo apt install -y libfdt-dev

SCRIPTDIR="$(realpath $(dirname $0))"

cd ~
if [ -d "raspberry-utils" ]
then
    echo "$(basename $0): destination dir already exists"
    exit
fi
git clone git@github.com:raspberrypi/utils.git
mv utils raspberry-utils -v
cd raspberry-utils
# patch valid at
# b7651d86d71a172b2208c67b2e360cbcb4f9d98f
# Jun 11 2025
git apply "$SCRIPTDIR/raspberry-utils.patch"

cmake -G Ninja .
ninja
