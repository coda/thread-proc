include $(foreign)/mkenv/common.mk

cflags += -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE

all: $(bld)/bin/runall exchg matmul alloc util

$(bld)/bin/runall: runall.sh

include util/gnu.mk
include alloc/gnu.mk
include matmul/gnu.mk
include exchg/gnu.mk
