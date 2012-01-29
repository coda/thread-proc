abld = $(bld)/alloc

asrc = thread.c proc.c worker.c
aobj = $(call c2o,$(abld),$(asrc))
abin = $(bld)/bin/at $(bld)/bin/ap

$(bld)/bin/at: $(ubld)/threadspawn.o $(ucommon)
$(bld)/bin/ap: $(ubld)/procspawn.o $(ucommon)

alloc: $(aobj)

include $(call o2d,$(aobj))
