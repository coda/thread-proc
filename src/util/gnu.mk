ubld = $(bld)/util

usrc = config.c echotwo.c tools.c spawn.c procspawn.c cfgtest.c spawntest.c
uobj = $(call c2o,$(ubld),$(usrc))
ubin = $(bld)/bin/cfgtest $(bld)/bin/procspawntest $(bld)/bin/threadspawntest
ucommon = $(call c2o,$(ubld),config.c echotwo.c tools.c spawn.c)

util: $(ubin)

$(bld)/bin/cfgtest: $(ubld)/cfgtest.o $(ucommon)
$(bld)/bin/procspawntest: $(ubld)/spawntest.o $(ubld)/procspawn.o $(ucommon)

$(bld)/bin/threadspawntest: lflags += -pthread
$(bld)/bin/threadspawntest: $(ubld)/spawntest.o $(ubld)/threadspawn.o $(ucommon)

include $(call o2d, $(uobj))
