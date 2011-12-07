#!/bin/bash

set -e

taskset -p $$

echo 2 1>&2
time ./mnt 2 2
time ./mnp 2 2

echo 8 1>&2

time ./mnt 2 8
time ./mnp 2 8

echo 512 1>&2
time ./mnt 2 512
time ./mnp 2 512
