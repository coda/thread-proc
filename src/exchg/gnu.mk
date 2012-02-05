ebld = $(bld)/exchg
esrc = proc.c ringtest.c heapsum.c ringlink.c util.c
eobj = $(call cpp2o,$(ebld),thread.cpp) $(call c2o,$(ebld),$(esrc))
# ebin = $(addprefix $(bld)/bin/,et ep)
ebin = $(addprefix $(bld)/bin/,rt)
ecommon = $(call c2o,$(ebld),heapsum.c ringlink.c) $(ucommon)

exchg: $(ebin)

$(bld)/bin/et: lflags += -lstdc++ -pthread
$(bld)/bin/et: $(ebld)/thread.o $(ecommon) $(ubld)/threadspawn.o

$(bld)/bin/ep: lflags += -lrt
$(bld)/bin/ep: $(ebld)/proc.o $(ecommon) $(ubld)/procspawn.o

$(bld)/bin/rt: lflags += -pthread
$(bld)/bin/rt: $(ebld)/ringtest.o $(ecommon) $(ubld)/threadspawn.o

include $(call o2d,$(eobj))
