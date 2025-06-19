# How To
```bash
./generate.sh
ninja
./CatPi
```

# TODO

- on quit: all driven pins to low
- document/reproduce the patch pinctrl exe2lib
- clang-format
- getting the weight from c++
- print the weight
- as a graphic widget
- with history: histogram?
- tare button
- another sub panel
- plug the relay from c++
- into a button
- logic "weight goes down <=> eat event"
- logic "dispense when eaten and/or dispense > 10 minutes away"

- settings
- logs
- icons

# DONE
- two horizonal split panels
- tool bar (vertical)
- button quit
- button fullscreen
- fixed size? size of pi screen?
-> automatic size, maximized or fullscreen on small screen
- gpio from c
```
do gpio from c -> relay before weight
- read doc https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
- try this https://superuser.com/a/1449936
device name in /sys/kernel/debug/pinctrl/pinctrl-devices
folder /sys/kernel/debug/pinctrl/*$devicename
read = grep gpio17 pins
write = TODO
maybe because there are few writes:
call pinctrl, like relay_off/on
port ~/bin/relay* to c++
port ~/examples/min.py:
  inline calls to calls gpiozero
port weight to c++
final choice: compile pinctrl
patch it to be a library
call it from c++
```
