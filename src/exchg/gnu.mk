ebld = $(bld)/exchg
esrc = proc.c heapsum.c util.c
eobj = $(call cpp2o,$(ebld),thread.cpp) $(call c2o,$(ebld),$(esrc))
ebin = $(addprefix $(bld)/bin/,et ep)
ecommon = $(call c2o,$(ebld),work.c util.c)

exchg: $(ebin)

$(bld)/bin/et: lflags += -lstdc++ -pthread
$(bld)/bin/et: $(ebld)/thread.o $(ecommon) $(ubld)/threadspawn.o

$(bld)/bin/ep: lflags += -lrt
$(bld)/bin/ep: $(ebld)/proc.o $(ecommon) $(ubld)/procspawn.o

include $(call o2d,$(eobj))
