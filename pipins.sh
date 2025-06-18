#!/bin/bash

# set -x
cd $(dirname $0)/src
clang pipins.c -llgpio -o pipins && ./pipins ; echo "returned $?"
