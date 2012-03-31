include $(foreign)/mkenv/gnu/common.mk

ifndef e2
e2 = $(bld)/F
$(warning e2 undefined; assuming $(e2))
endif

cflags += -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE -I $(e2)/include
lflags += -L $(e2)/lib -le2

all: $(bld)/bin/runall util alloc exchg matmul

$(bld)/bin/runall: runall.sh

include util/gnu.mk
include alloc/gnu.mk
include matmul/gnu.mk
include exchg/gnu.mk
