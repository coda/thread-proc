abits := $(call bitspath)

asrc = thread.c proc.c worker.c
aobj = $(call c2o,$(abits),$(asrc))
abin = $(bld)/bin/at $(bld)/bin/ap
acommon = $(abits)/worker.o $(L)/libucm.a

alloc: $(abin)

$(bld)/bin/at: lflags += -pthread
$(bld)/bin/at: $(abits)/thread.o $(L)/libust.a $(acommon) 

$(bld)/bin/ap: $(abits)/proc.o $(L)/libusp.a $(acommon) 

include $(call o2d,$(aobj))
