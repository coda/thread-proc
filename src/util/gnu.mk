lbld = $(bld)/util

lsrc = \
	config.c \
	echotwo.c

lobj = $(call c2o, $(lbld), $(lsrc))

$(bld)/lib/libutil.a: $(lobj)

include $(call o2d, $(lobj))
