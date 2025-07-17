# How To
```bash
./generate.sh
ninja
./CatPi
```

# TODO

- signal of weight is noisy
  it easily goes over threshold because of spikes
  average, median: research a bit
  still display both the raw and the processed

- logic: dont dispense right after eating, have a delay
  needs to detect eat events
  which needs a reliable weight signal
  which needs the processing

- why not do auto-tare at this moment:
  auto-tare = when we detect it's empty and we should dispense
  because then, we want to detect "dispensed portion is eaten"
  so it's really "weight increase when we served went back"

- logic: bug sometimes dispense zero or very little: detect and dispense again
  needs to detect eat events

- have interia in the Dial
  so that it's easy to use for small and big changes

- logging
  log eat events and dispense events
  log weight signal raw and processed
  rotate / compress the logs for disk

- plot, histograms
  eat events
  dispense events
  dispensed grams (needs reliable weight signal)

- status message (tare, calib result) is off screen (resizes window)
  reproduced on wsl -> detect window resize more than pi screen size
  fix =
  make window not resizable
  print on detect window resized
  either make permanent room for status message
  xor display it over something
  xor display it smaller

- libhx711: what happens when unplugged? crashes
 - start unplugged
 - unplug/replug multiple times
 - start unplugged then plug (multiple times)
 - start in buggy state

- libhx711: can we take out the implementation and do it in tree?

- base widget: maybe QSlider is better than DeltaDial ?

- more debug settings:
  - load cell polling, refresh frequency : to test
  - load cell polling, timeout (for cpu consumption when unplugged): to test
  - duration of button/relay press (<-> quantity)
  - minimum weight for bowl is empty
  - two pins of HX711
  - gain?
  - duration of confirm tare

- disable dpi scaling on linux side for emacs and CatPi

- IR motion detector: can it do auto on/off of the screen?

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
- close relay even on crash
  needs crash handler
  so then just port all features
  handle control-c: also close relay
  but on control-c: do not print stack trace, it's user-initiated SIGINT
- halt on nan and inf
- problem: the screen is too bright in the night
  test screen saver shuts off screen:
  can it be waken up with a touch? yes
  can we control the screen brigthness in-app? yes
- screen saver:
  detect even if it hits a child widget
  remove debug print in ScreenBrightness::setIsOn()
  reset brightness to default on quit
  do not pass event if screen is off : no, pass it, it's too complex to inhibit
    instead, protect the buttons with long-press requirement
  remove debug setting delay is seconds instead of minutes
- do not allow multiple instances
- pi emacs: sync config, enable lsp
  test 5 bindings: Alt/Control * P/O, Alt-backquote
  remove all .elc from repo
  remove links that make duplicate .el
  auto compile elc
  fix compile error   > Error (bytecomp): Bytecode overflow
  on pi, clone dotfiles anonymously
  on pi, hide git status in home
  fewer threads for clangd if nproc 4
- emacs:
 compile: find root dir with locate-dominating-file
 do not prompt for compile command
 allow change compile command
 fix the ctrl-b recompile goes to ripgrep ?
 auto jump to first error, but compile only, not ripgrep
   maybe like this:
   setq compilation-last-buffer nil
   on this hook
   rg-finish-function
 visit buffer rg on results: rg-finish-functions
 search current word: see isearch-forward-symbol-at-point
https://irfu.cea.fr/Pisp/frederic.galliano/Computing/manual_elisp.html
 select current word, line
- crash handler compatibility on pi: stacktrace is broken
  use glibc backtrace
  handle conditional paths for arm and x86
  demangle c++ abi
  maybe needs f omit frame pointer for my code
  parse this new syntax in our crash dialog
  add to generate.sh: binutils
- use object or source file, compared to application filepath
  to not leak personal info
  and to compute is user code
- test inf and nan on arm
  maybe it's not supported
  https://stackoverflow.com/a/77334141
  https://community.nxp.com/t5/i-MX-Processors/No-exception-when-divided-by-zero/m-p/1793764
  https://stackoverflow.com/a/79018956
  https://developer.arm.com/documentation/dui0808/l/Floating-point-Support/Controlling-the-Arm-floating-point-environment
- emacs lsp-treemacs-errors-list does not work great, find better
- write crash log to a file
- also long-press-protect the DispenseNow button
  because wake screen from sleep does press buttons
  with a shared widget: heritor of push button and composer of progress bar
  so that we can find the signals
  but then it won't work for toolbar which has tool buttons that do not inherit from push button
  so inherit from abstract button? but no, we want to instantiate push and tool buttons
- no need for pragma once when we just declare
- tare progress bar moves other widgets around! "grams"
  what about the progress bar vertical on the side of the button? -> bad
  make it always visible, disabled when not in use, transparent when disabled
- better gui layout
  frequent buttons bigger
- Dial: bigger maximum for smooth rotate, then divide delta
  does not work: some composers of Dial change the maximum after construction
  lower-than-default maximum (for Delay) is more precise but very not smooth
  -> use the default maximum that is quite smooth
  give more screen space to the Delay dial: is it smooth now?
- translate all labels to french
- labels Delay and Remaining: bigger? but where!
