/* #include <linux/gpio.h> */
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
/* #include <sys/ioctl.h> */
/* #include <fcntl.h> // open(), O_RDONLY */
/* #include <unistd.h> // close() */

#include <lgpio.h>

char* level_to_string(int level) {
  if (level == 0) { return "lo"; }
  if (level == 1) { return "hi"; }
  return "er";
}

int has_bit(mode, bit) { return mode & (1 << bit); }

char* mode_to_string(int mode) {
  for (int bit=0; bit<=19; ++bit) {
    if (!has_bit(mode, bit)) { continue; }
    switch(bit) {
    case 0: return "Kernel: In use by the kernel";
    case 1: return "Kernel: Output";
    case 2: return "Kernel: Active low";
    case 3: return "Kernel: Open drain";
    case 4: return "Kernel: Open source";
    case 5: return "Kernel: Pull up set";
    case 6: return "Kernel: Pull down set";
    case 7: return "Kernel: Pulls off set";
    case 8: return "LG: Input";
    case 9: return "LG: Output";
    case 10: return "LG: Alert";
    case 11: return "LG: Group";
    case 12: return "LG: ---";
    case 13: return "LG: ---";
    case 14: return "LG: ---";
    case 15: return "LG: ---";
    case 16: return "Kernel: Input";
    case 17: return "Kernel: Rising edge alert";
    case 18: return "Kernel: Falling edge alert";
    case 19: return "Kernel: Realtime clock alert";
    }
  }
  return "";
}

int main(int argc, char** argv) {
  /* /\* FILE* f = fopen("/dev/gpiochip0", "w"); *\/ */
  /* /\* printf("f %p\n", f); *\/ */
  /* /\* int fd = fileno(f); *\/ */
  /* int fd = open("/dev/gpiochip0", O_RDONLY); */
  /* printf("fd %d\n", fd); */
  /* /\* if (fd) { return 1; } *\/ */

  /* struct gpiohandle_request req = { */
  /*   .lines = 1, */
  /*   .lineoffsets = {0}, */
  /*   .flags = GPIOHANDLE_REQUEST_OUTPUT, */
  /*   .default_values = {1}, */
  /*   .consumer_label = "my-app", */
  /* }; */

  /* ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req); */

  // from `man lgpio`

  int h = lgGpiochipOpen(0); // open /dev/gpiochip0
  printf("chip %d\n", h);

  if (h >= 0)
  {
    // open ok
  }
  else
  {
    // open error
    printf("ERROR: %s (%d)\n", lguErrorText(h), h);
    return 1;
  }

  int status = LG_OKAY;
  

  lgChipInfo_t cInfo;

  status = lgGpioGetChipInfo(h, &cInfo);

  if (status == LG_OKAY)
  {
    printf("lines=%d name=%s label=%s\n",
           cInfo.lines, cInfo.name, cInfo.label);
  }


  lgLineInfo_t lInfo;

  int is_gpio[cInfo.lines];

  /* lgGpioSetUser(h, "foo"); */
  
  for (int gpio=0; gpio<cInfo.lines; ++gpio) {
    status = lgGpioGetLineInfo(h, gpio, &lInfo);

    if (status == LG_OKAY)
    {
      is_gpio[gpio] = !strncmp(lInfo.name, "GPIO", 4);
      if (is_gpio[gpio]) {
        lgGpioClaimInput(h, 0, gpio);
        /* printf("[%d] lFlags=%d name=%s user=%s level=%s\n", */
        /*        gpio, lInfo.lFlags, lInfo.name, lInfo.user, */
        char* gap = (gpio < 10) ? " " : "";
        if (gpio < 10) { printf("%s", gap); }
        printf("[%d] %s %s %s %d %s\n",
               gpio,
               lInfo.name,
               gap,
               "", -1, "");
               //level_to_string(lgGpioRead(h, gpio)),
               //lgGpioGetMode(h, gpio),
               //mode_to_string(lgGpioGetMode(h, gpio)));
        lgGpioFree(h, gpio);
      }
    } else {
      is_gpio[gpio] = 0;
      printf("ERROR: %s (%d)\n", lguErrorText(h), h);
    }
  } 
  
  return 0;
}
