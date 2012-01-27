ubld = $(bld)/util

usrc = config.c echotwo.c procspawn.c cfgtest.c spawntest.c

uobj = $(call c2o,$(ubld),$(usrc))
ucommon = $(call c2o,$(ubld),config.c echotwo.c)

libutil: $(bld)/bin/cfgtest $(bld)/bin/procspawntest

$(bld)/bin/cfgtest: $(ubld)/cfgtest.o $(ucommon)
$(bld)/bin/procspawntest: $(ubld)/spawntest.o $(ubld)/procspawn.o $(ucommon)

include $(call o2d, $(uobj))
