mbld = $(bld)/matmul
msrc = naivemul.c tilemul.c muljob.c util.c thread.c proc.c proconfs.c
mobj = $(call c2o,$(mbld),$(msrc))
# mbin = $(addprefix $(bld)/bin/,mnt mtt mnp mtp mnpf mtpf)
mbin = $(addprefix $(bld)/bin/,mnt mtt)

mpcommon = $(ucommon) $(ubld)/procspawn.o
mtcommon = $(ucommon) $(ubld)/threadspawn.o

# naive = $(call c2o, $(mbld),util.c naivemul.c)
# tile = $(call c2o, $(mbld),util.c tilemul.c)

naive = $(call c2o,$(mbld),naivemul.c muljob.c)
tile = $(call c2o,$(mbld),tilemul.c muljob.c)

matmul: $(mbin)

$(bld)/bin/mnt: $(mbld)/thread.o $(naive) $(mtcommon)
$(bld)/bin/mtt: $(mbld)/thread.o $(tile) $(mtcommon)
 
$(bld)/bin/mnp: $(mbld)/proc.o $(naive)
$(bld)/bin/mtp: $(mbld)/proc.o $(tile)
 
$(bld)/bin/mnpf: $(mbld)/proconfs.o $(naive)
$(bld)/bin/mtpf: $(mbld)/proconfs.o $(tile)

include $(call o2d,$(mobj))
