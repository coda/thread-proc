ubld = $(bld)/util

usrc = \
	config.c \
	echotwo.c \
	cfgtest.c

uobj = $(call c2o, $(ubld), $(usrc))

libutil: $(bld)/bin/cfgtest $(uobj)

$(bld)/bin/cfgtest: $(ubld)/cfgtest.o $(uobj)

include $(call o2d, $(uobj))
