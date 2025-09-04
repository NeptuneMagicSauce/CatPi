# How To
```bash
./generate.sh
ninja
./CatPi
```


# TODO

- plots ...

- to validate, test this exploit:
  eat right away
  isn't it detected as a mechanical issue and doesn't it repeat the dispense?

- to validate: is loading historical data slow on boot?


- mask LEDs that are too bright in the night

- needs cord extension, une rallonge

- needs auto update button in the GUI

- auto run on boot

- update capability: in the GUI?
  it needs to be bullet-proof for reverting broken updates
  maybe dedicated external gui:
  list commits of branch main
  list branches
  or: use a gui for git, then compile

- `upgrade_cat_pi`:
  git fetch to get new tags
  git stash if dirty
  git pull --rebase if force pushed

- when cat eats just after dispense,
  it pushes on the scale
  and the scale detects too much
  as high as 4.0 grams

- counter weight behind for no tipping-over

- verify calibration with the independent source of truth

- have a maximum weight per day
  maybe 55 grams (a changeable setting)
  but using the sliding window total
  so that the clamping is smoothed over time

- go out of night mode automatically
  into day mode
  maybe at 6:30 AM (a changeable setting)
  but then, it's not "day/night buttons"
  it's "day/night modes"
  -> not a default push button, but with property checkable ?!

# TODO but later for V2

- auto restart on crash
  but not on quit
  and not forever: max X times per Y period

- frequencies, performance:
  is load cell updating really only every 1000 milliseconds?
  how is increasing the logic frequency making a more accurate weight measure then?
  do not update the GUI as frequently, it only needs every 1 second
  <-> do not update the Logic as frequently

- computing the dispensed weight is slightly inaccurate
  because it is the maximum seen in the 10 seconds following a dispense
  and sometimes it stabilizes a bit below the maximum, like 0.1 gram
  so fix would be maybe "weight at dispense + 10 seconds"
  no because it can be eaten already

- compress previous log file, when it's a new day (new log file)

- log weight signal raw and processed, to check noise is inhibited

- plot the dispense and eat events in a sub screen
  or in a web page
  with the total weight per day and per 24 hours

- bug A: sometimes it dispenses soon after starting
  maybe after fiddling with the delay timer?
  or is it working as expected?
  because when we decrease the delay, we can trigger a dispense event!

- have inertia in the Dial
  so that it's easy to use for small and big changes

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
- bug: when screen saver is on (screen turned off)
  waking by touching does fire the buttons
  fix = set MainWindow QWidget::disabled when screen saver is on
- signal of weight is noisy
  it easily goes over threshold because of spikes
  with the median algorithm
- why not do auto-tare at this moment:
  auto-tare = when we detect it's empty and we should dispense
  because then, we want to detect "dispensed portion is eaten"
  so it's really "weight increase when we served went back"
- status message (tare, calib result) is off screen (resizes window)
  reproduced on wsl -> detect window resize more than pi screen size
  fix =
  make window not resizable
  print on detect window resized
  either make permanent room for status message
  xor display it over something
  xor display it smaller
  xor do not display status messages
- detect eat events
- logging
  log eat events
  log dispense events
  one log file per day (or hour)
  log dispensed weight
  per day, per 24 hours : number of events and weight in the GUI
- debug setting minWeightThreshold
- logic: dont dispense right after eating, have a delay
  needs to detect eat events
  which needs a reliable weight signal
  which needs the processing
- timer is now incorrect in GUI
  because time threshold depends on time of eating
  fix is:
  start delay timer on eat event
  do not use "time since eating" in Logic
  but "delay is finished"
  and rename "elapsedSecondsSinceDispense" to something more relevant
  do auto-tare on start
   button tare in debug settings is borken
  because tare print in debug is too big
  also it overflows the screen
  time to dispense goes negative when it's not empty (but eaten!?)
  should it dispense because eaten and waited
  or should it not because not empty
  yes dispense, because the reasons we can be in this situation imply yes:
    if more croquette were added physically, with the hands
    if the balance bowl moved around and 'changed' its weight
    if the load sensor is too noisy
- center buttons on the left, in menu screen
- on event eat: log measured weight
  and recent values also
  to debug why it detects event eat
- consider justAte if weight has been below threshold for a bit of time
- log dispensed weight
- redo layout main/menu
- have a Setting: duration of motor rotating!
- bug B: sometimes dispense zero or very little
  detect it and dispense again
- if screen is sleeping and app is not active -> can not go out of fullscreen
- load old logs from file, do not have only logs from runtime
- needs detailed logs, last 6 events is not enough
  loading historical data on demand
  have labels on both axis and title
