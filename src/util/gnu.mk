ubld = $(bld)/util

usrc = config.c echotwo.c spawn.c procspawn.c cfgtest.c spawntest.c

uobj = $(call c2o,$(ubld),$(usrc))
ucommon = $(call c2o,$(ubld),config.c echotwo.c spawn.c)

libutil: $(bld)/bin/cfgtest $(bld)/bin/procspawntest $(bld)/bin/threadspawntest

$(bld)/bin/cfgtest: $(ubld)/cfgtest.o $(ucommon)
$(bld)/bin/procspawntest: $(ubld)/spawntest.o $(ubld)/procspawn.o $(ucommon)
$(bld)/bin/threadspawntest: $(ubld)/spawntest.o $(ubld)/threadspawn.o $(ucommon)

include $(call o2d, $(uobj))
