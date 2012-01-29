abld = $(bld)/alloc

asrc = thread.c proc.c worker.c
aobj = $(call c2o,$(abld),$(asrc))

alloc: $(aobj)

include $(call o2d,$(aobj))
