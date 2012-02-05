mbld = $(bld)/matmul
msrc = naivemul.c tilemul.c element.c muljob.c thread.c proc.c proconfs.c
mobj = $(call c2o,$(mbld),$(msrc))
mbin = $(addprefix $(bld)/bin/,mnt mtt mnp mtp mnpf mtpf)

mtcommon = $(ucommon) $(ubld)/threadspawn.o
mpcommon = $(ucommon) $(ubld)/procspawn.o $(ubld)/memmap.o
mpfcommon = $(mpcommon) $(ubld)/memfile.o

# naive = $(call c2o, $(mbld),util.c naivemul.c)
# tile = $(call c2o, $(mbld),util.c tilemul.c)

naive = $(call c2o,$(mbld),naivemul.c element.c muljob.c)
tile = $(call c2o,$(mbld),tilemul.c element.c muljob.c)

matmul: $(mbin)

$(bld)/bin/mnt \
$(bld)/bin/mtt: lflags += -pthread
$(bld)/bin/mnt: $(mbld)/thread.o $(naive) $(mtcommon)
$(bld)/bin/mtt: $(mbld)/thread.o $(tile) $(mtcommon)
 
$(bld)/bin/mnp: $(mbld)/proc.o $(naive) $(mpcommon)
$(bld)/bin/mtp: $(mbld)/proc.o $(tile) $(mpcommon)
 
$(bld)/bin/mnpf \
$(bld)/bin/mtpf: lflags += -lrt
$(bld)/bin/mnpf: $(mbld)/proconfs.o $(naive) $(mpfcommon)
$(bld)/bin/mtpf: $(mbld)/proconfs.o $(tile) $(mpfcommon)

include $(call o2d,$(mobj))
