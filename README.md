# How To
```bash
./generate.sh
ninja
./CatPi
```

# TODO

- libhx711: what happens when unplugged? crashes
 - start unplugged
 - unplug/replug multiple times
 - start unplugged then plug (multiple times)
 - start in buggy state
- libhx711: can we take out the implementation and do it in tree?
- libhx711: is there a tare function?
- libhx711: implement calibration
- libhx711: catch all exceptions same as samples: TimeOut, see all bins

- weight graphic widget: progress bar or circular dial?
- with history: histogram?
- tare button (saved and restored)

- on quit: all driven pins to low

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
- another sub panel
- plug the relay from c++
- into a button
- gpio from c
do gpio from c -> relay before weight
  read doc https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
  try this https://superuser.com/a/1449936
  device name in /sys/kernel/debug/pinctrl/pinctrl-devices
  folder /sys/kernel/debug/pinctrl/$devicename
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
- document/reproduce the patch pinctrl exe2lib
- move to smaller files for compile performance on pi
- clang-format
- getting the weight from c++
- print the weight
- set up v19 for all 3 clangs: clang format clangd
- libhx711: what about setting another gain? (including for calibration?)
  measures look very similar with 64 and 128
  128 should be more precise because numeric range is greater (scale is greater)
- libhx711: start plugged then unplug (multiple times) -> busy wait
- libhx711: fix the busy wait
