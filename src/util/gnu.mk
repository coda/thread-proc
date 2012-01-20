lbld = $(bld)/util

lsrc = \
	config.c \
	echotwo.c \
	cfgtest.c

lobj = $(call c2o, $(lbld), $(lsrc))

libutil: $(bld)/bin/cfgtest $(lobj)

$(bld)/bin/cfgtest: $(lbld)/cfgtest.o $(lobj)

include $(call o2d, $(lobj))
