#!/bin/bash

set -e

taskset -p $$

echo 1 1>&2
time ./mnt 2 1
time ./mnp 2 1
time ./mnpm 2 1

echo 2 1>&2
time ./mnt 2 2
time ./mnp 2 2
time ./mnpm 2 2

echo 8 1>&2
time ./mnt 2 8
time ./mnp 2 8
time ./mnpm 2 8

echo 512 1>&2
time ./mnt 2 512
time ./mnp 2 512
time ./mnpm 2 512
