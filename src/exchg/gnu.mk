ebld = $(bld)/exchg

esrc = proc.c heapsum.c ringlink.c util.c nixwrapper.c \
	ringtest-proc.c ringtest-thread.c

eobj = $(call cpp2o,$(ebld),thread.cpp) $(call c2o,$(ebld),$(esrc))
# ebin = $(addprefix $(bld)/bin/,et ep)
ebin = $(addprefix $(bld)/bin/,rtt rtp et)
ecommon = $(call c2o,$(ebld),heapsum.c ringlink.c nixwrapper.c) $(ucommon)

exchg: $(ebin)

$(bld)/bin/et: lflags += -lstdc++ -pthread
$(bld)/bin/et: $(ebld)/thread.o $(ecommon) $(ubld)/threadspawn.o

$(bld)/bin/ep: lflags += -lrt
$(bld)/bin/ep: $(ebld)/proc.o $(ecommon) $(ubld)/procspawn.o

$(bld)/bin/rtt: lflags += -pthread
$(bld)/bin/rtt: $(ebld)/ringtest-thread.o $(ecommon) $(ubld)/threadspawn.o

$(bld)/bin/rtp: $(ebld)/ringtest-proc.o $(ecommon) $(ubld)/procspawn.o

include $(call o2d,$(eobj))
