ubld = $(bld)/util

usrc = config.c echotwo.c tools.c memfile.c memmap.c \
	procspawn.c threadspawn.c \
	cfgtest.c spawntest.c

uobj = $(call c2o,$(ubld),$(usrc))
ubin = $(bld)/bin/cfgtest $(bld)/bin/procspawntest $(bld)/bin/threadspawntest
ucommon = $(call c2o,$(ubld),config.c echotwo.c tools.c)

umemcommon = $(ucommon) $(call c2o,$(ubld),memmap.c memfile.c)

util: $(ubin)

$(bld)/bin/cfgtest: $(ubld)/cfgtest.o $(ucommon)

$(bld)/bin/procspawntest: $(ubld)/spawntest.o $(ubld)/procspawn.o $(ucommon)

$(bld)/bin/threadspawntest: lflags += -pthread
$(bld)/bin/threadspawntest: $(ubld)/spawntest.o $(ubld)/threadspawn.o $(ucommon)

include $(call o2d, $(uobj))
