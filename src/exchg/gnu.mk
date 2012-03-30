ebld := $(call bitspath)

esrc = heapsum.c vector.c ringlink.c nixwrapper.c \
	ringtest-proc.c ringtest-thread.c \
	proc.c 

eobj = $(call cpp2o,$(ebld),thread.cpp) $(call c2o,$(ebld),$(esrc))
ebin = $(addprefix $(bld)/bin/,rtt rtp et ep)

ecommon = $(call c2o,$(ebld),heapsum.c ringlink.c nixwrapper.c) $(ucommon)
epcommon = $(ecommon) $(ebld)/vector.o

exchg: $(ebin)

$(bld)/bin/et: lflags += -lstdc++ -pthread
$(bld)/bin/et: $(ebld)/thread.o $(ecommon) $(uthrdspawn)

$(bld)/bin/ep: lflags += -lrt
$(bld)/bin/ep: $(ebld)/proc.o $(epcommon) $(uprocspawn) $(umemcommon)

$(bld)/bin/rtt: lflags += -pthread
$(bld)/bin/rtt: $(ebld)/ringtest-thread.o $(ecommon) $(uthrdspawn)

$(bld)/bin/rtp: $(ebld)/ringtest-proc.o $(ecommon) $(uprocspawn)

include $(call o2d,$(eobj))
