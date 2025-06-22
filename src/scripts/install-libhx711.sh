set -e

sudo apt install -y liblgpio-dev
SCRIPTDIR="$(realpath $(dirname $0))"

cd ~
if [ -d libhx711 ]
then
    exit
fi

git clone --depth=1 https://github.com/endail/hx711

mv hx711 libhx711
cd libhx711

# patch valid at
# ed1da427e61d371b69ccad29f7b816243cd7299d
# Jan 22, 2024
git apply "$SCRIPTDIR/libhx711.patch"

mkdir -p build/shared
mkdir -p build/static
mkdir -p bin

for i in src/*cpp
do
    j=$(basename $i|sed 's/cpp/o/')
    make build/static/$j build/shared/$j
done

make build/shared/libhx711.so
make build/static/libhx711.a

make test
make hx711calibration
