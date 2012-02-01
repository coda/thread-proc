include $(foreign)/mkenv/common.mk

cflags += -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE
lflags += -pthread -lrt

all: matmul alloc util

include util/gnu.mk
include alloc/gnu.mk
include matmul/gnu.mk

