# How To
```bash
./generate.sh
ninja
./CatPi
```

# TODO

- close relay even on crash
  needs crash handler
  so then just port all features
  handle control-c: also close relay
  but on control-c: do not print stack trace, it's user-initiated SIGINT

- more debug settings:
  - load cell polling, refresh frequency : to test
  - load cell polling, timeout (for cpu consumption when unplugged): to test
  - duration of button/relay press (<-> quantity)
  - minimum weight for bowl is empty
  - two pins of HX711
  - gain?
  - duration of confirm tare

- problem: the screen is too bright in the night
  test screen saver shuts off screen:
  can it be waken up with a touch?
  can we control the screen brigthness in-app?
  IR motion detector: can it do auto on/off of the screen?

- better gui layout
  frequent buttons bigger

- also long-press-protect the DispenseNow button ?
  with a shared widget: heritor of push button and composer of progress bar
  so that we can find the signals
  but then it won't work for toolbar which has tool buttons that do not inherit from push button
  so inherit from abstract button? but no, we want to instantiate push and tool buttons

- Dial: bigger maximum for smooth rotate, then divide delta
  does not work: some composers of Dial change the maximum after construction
  lower-than-default maximum (for Delay) is more precise but very not smooth
  -> use the default maximum that is quite smooth
  give more screen space to the Delay dial: is it smooth now?

- signal of weight is noisy
  it easily goes over threshold because of spikes
  average, median: research a bit
  still display both the raw and the processed

- logic: dont dispense right after eating, have a delay
  needs to detect eat events
  which needs a reliable weight signal
  which needs the processing

- logic: bug sometimes dispense zero or very little: detect and dispense again
  needs to detect eat events

- libhx711: what happens when unplugged? crashes
 - start unplugged
 - unplug/replug multiple times
 - start unplugged then plug (multiple times)
 - start in buggy state

- libhx711: can we take out the implementation and do it in tree?

- base widget: maybe QSlider is better than DeltaDial ?

- logging
  log eat events and dispense events
  log weight signal raw and processed
  rotate / compress the logs for disk

- plot, histograms
  eat events
  dispense events
  dispensed grams (needs reliable weight signal)

- disable dpi scaling on linux side for emacs and CatPi

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
  write = todo
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
- libhx711: implement a tare function
- keyboard shortcuts: fullscreen and quit
- calibration: screens for 2 measures
- widget to input precise known weight
- libhx711: implement calibration
- tare button (saved and restored)
- logic "weight goes down <=> eat event"
- logic "dispense when eaten and/or dispense > 10 minutes away"
- NO: weight graphic widget: progress bar or circular dial?
- libhx711: catch all exceptions same as samples: TimeOut, see all bins
- icons: nicer
- optional: use value() because it checks, not operators * or -> because they don't check
- icon back: bigger, nicer
- rename wait widgets
- settings menu to easy-change in dedicated menu
- button to reset calibration to factory defaults
  because if you go through the calibration screens
  but you don't have the known weight
  but you put your finger on the scale
  then it's in a state with bad calibration data
  in the debug menu?
- clang format:
  - no // namespace at closure
  - maybe width smaller
  - why is the QList<SubScreen> so bizarrely indented?
- clang-tidy: include cleaner: warn on unused includes: already working
- decouple logic loop from sampling loop
- tick LoadCell and Logic right away, dont wait a first interval
  so that all data is displayed right away: weight and delay
  NO, too complex, instead :
  init the gui before looping with correct values
  before weight data is available, or when it's not: display "--"
- debug setting: refresh frequency of the main loop
