mbits := $(call bitspath)

msrc = naivemul.c tilemul.c element.c muljob.c thread.c proc.c proconfs.c
mobj = $(call c2o,$(mbits),$(msrc))
mbin = $(addprefix $(B)/,mnt mtt mntm mttm mnp mtp mnpf mtpf)
mlib = $(addprefix $(L)/,libmx.a)

mtcommon = $(L)/libust.a $(L)/libucm.a
mpcommon = $(L)/libusp.a $(L)/libucm.a

matmul: $(mbin) $(mlib)

$(L)/libmx.a: $(call c2o,$(mbits),element.c muljob.c)

naive = $(L)/libmx.a $(mbits)/naivemul.o
tile = $(L)/libmx.a $(mbits)/tilemul.o

$(B)/mnt \
$(B)/mtt: lflags += -pthread
$(B)/mnt: $(mbits)/thread.o $(naive) $(mtcommon)
$(B)/mtt: $(mbits)/thread.o $(tile) $(mtcommon)

$(B)/mntm \
$(B)/mttm: lflags += -pthread
$(B)/mntm: $(mbits)/threadonmap.o $(naive) $(mtcommon)
$(B)/mttm: $(mbits)/threadonmap.o $(tile) $(mtcommon)
 
$(B)/mnp: $(mbits)/proc.o $(naive) $(mpcommon)
$(B)/mtp: $(mbits)/proc.o $(tile) $(mpcommon)
 
$(B)/mnpf \
$(B)/mtpf: lflags += -lrt
$(B)/mnpf: $(mbits)/proconfs.o $(naive) $(mpcommon)
$(B)/mtpf: $(mbits)/proconfs.o $(tile) $(mpcommon)

include $(call o2d,$(mobj))
