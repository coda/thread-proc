ubits := $(call bitspath)

usrc = \
	config.c tools.c memfile.c memmap.c \
	procspawn.c threadspawn.c \
	cfgtest.c spawntest.c

uobj = $(call c2o,$(ubits),$(usrc))
ubin = $(addprefix $(B)/,cfgtest procspawntest threadspawntest)
ulib = $(addprefix $(L)/,libucm.a libust.a libusp.a)

util: $(ubin) $(ulib)

$(B)/cfgtest: $(ubits)/cfgtest.o $(L)/libucm.a

$(B)/procspawntest: $(ubits)/spawntest.o $(L)/libusp.a $(L)/libucm.a 

$(B)/threadspawntest: lflags += -pthread
$(B)/threadspawntest: $(ubits)/spawntest.o $(L)/libust.a $(L)/libucm.a

$(L)/libucm.a: $(call c2o,$(ubits),config.c tools.c memmap.c memfile.c)

$(L)/libust.a: $(ubits)/threadspawn.o
$(L)/libusp.a: $(ubits)/procspawn.o

include $(call o2d, $(uobj))
