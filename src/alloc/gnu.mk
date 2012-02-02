abld = $(bld)/alloc

asrc = thread.c proc.c worker.c
aobj = $(call c2o,$(abld),$(asrc))
abin = $(bld)/bin/at $(bld)/bin/ap
acommon = $(abld)/worker.o $(ucommon)

alloc: $(abin)

$(bld)/bin/at: lflags += -pthread
$(bld)/bin/at: $(abld)/thread.o $(acommon) $(ubld)/threadspawn.o

$(bld)/bin/ap: $(abld)/proc.o $(acommon) $(ubld)/procspawn.o

include $(call o2d,$(aobj))
