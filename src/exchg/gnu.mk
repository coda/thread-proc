ebits := $(call bitspath)

esrc = heapsum.c vector.c ringlink.c nixwrapper.c \
	ringtest-proc.c ringtest-thread.c \
	proc.c 

eobj = $(call cpp2o,$(ebits),thread.cpp) $(call c2o,$(ebits),$(esrc))
ebin = $(addprefix $(B)/,rtt rtp et ep)
elib = $(addprefix $(L)/,libxch.a)

ecommon =  $(elib) $(L)/libucm.a
epcommon = $(ecommon) $(ebits)/vector.o

exchg: $(ebin) $(elib)

$(L)/libxch.a: $(call c2o,$(ebits),heapsum.c ringlink.c nixwrapper.c)

$(B)/et: lflags += -lstdc++ -pthread
$(B)/et: $(ebits)/thread.o $(L)/libust.a $(ecommon)

$(B)/ep: lflags += -lrt
$(B)/ep: $(ebits)/proc.o $(L)/libusp.a $(epcommon)

$(B)/rtt: lflags += -pthread
$(B)/rtt: $(ebits)/ringtest-thread.o $(L)/libust.a $(ecommon)

$(B)/rtp: $(ebits)/ringtest-proc.o $(L)/libusp.a $(ecommon)

include $(call o2d,$(eobj))
