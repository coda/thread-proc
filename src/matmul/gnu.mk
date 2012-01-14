mmbld = $(bld)/matmul

mmsrc = \
	naivemul.c \
	tilemul.c \
	util.c \
	thrd.c \
	proc.c \
	proconfs.c

naive = $(call c2o, $(mmbld), util.c naivemul.c)
tile = $(call c2o, $(mmbld), util.c tilemul.c)

matmul: $(addprefix $(bld)/bin/, mnt mtt mnp mtp mnpf mtpf)

$(bld)/bin/mnt: $(mmbld)/thrd.o $(naive)
# 	@ echo -e '\tlnk\t$@' \
# 	&& $(lnk) $(lflags) $^ -pthread -o $@
 
$(bld)/bin/mtt: $(mmbld)/thrd.o $(tile)
# 	@ echo -e '\tlnk\t$@' \
# 	&& $(lnk) $(lflags) $^ -pthread -o $@
 
$(bld)/bin/mnp: $(mmbld)/proc.o $(naive)
# 	@ echo -e '\tlnk\t$@' \
# 	&& $(lnk) $(lflags) $^ -o $@
 	
$(bld)/bin/mtp: $(mmbld)/proc.o $(tile)
# 	@ echo -e '\tlnk\t$@' \
# 	&& $(lnk) $(lflags) $^ -o $@
 
$(bld)/bin/mnpf: $(mmbld)/proconfs.o $(naive)
# 	@ echo -e '\tlnk\t$@' \
# 	&& $(lnk) $(lflags) $^ -lrt -o $@
 
$(bld)/bin/mtpf: $(mmbld)/proconfs.o $(tile)
# 	@ echo -e '\tlnk\t$@' \
# 	&& $(lnk) $(lflags) $^ -lrt -o $@

include $(call o2d, $(call c2o, $(mmbld), $(mmsrc)))
