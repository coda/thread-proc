set -e

# niter=4
# nworkersupto=32
# 
# [ "$1" == '' ] || nworkersupto="$1"
# [ "$2" == '' ] || niter="$2"
# 
# for((nw=1; nw <= nworkersupto; nw <<= 1))
# do
# 	ptime=$({ time ./pt $(($niter * 1024 * 1024)) $nw; } 2>&1 \
# 		| awk '/^real.*/ {print $2}')
# 
# 	ttime=$({ time ./tt $(($niter * 1024 * 1024)) $nw; } 2>&1 \
# 		| awk '/^real.*/ {print $2}')
# 
# 	printf $niter"M / %- 4u\tT: $ttime\tP: $ptime\n" $nw
# done

# echo $@
# shift
# echo $@
# 
# for i in "$@"; do echo $i; done

prgs=()

UPTO=1024

while [ "$1" != '--' ]
do
	prgs[${#prgs[@]}]="$1"
	shift
done

shift
args=("$@")

# for i in "${prgs[@]}"; do echo $i; done
# for i in "${args[@]}"; do echo $i; done


for((nw = 1; nw <= $UPTO; nw <<= 2))
do
	exectimes=()

	for i in "${prgs[@]}"
	do
# 		cmd="'$nw'"
# 		for((j = 0; j < ${args[@]}; j += 1))
# 		do
# 			cmd="$cmd '${args[j]}'"
# 		done

		echo "./$i $nw ${args[@]}" 1>&2
			
		t=$({ time ./$i $nw ${args[@]}; } 2>&1 > /dev/null \
			| awk '/^real.*/ {print $2}')
 		
		exectimes[${#exectimes[@]}]="$t"
	done

	echo -n "$nw"
	for((i = 0; i < ${#prgs[@]}; i += 1))
	do
		echo -ne "\t${prgs[i]}: ${exectimes[i]}"
	done
	echo
done	
