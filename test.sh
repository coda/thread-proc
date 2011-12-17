set -e

prgs=()

UPTO=1024

while [ "$1" != '--' ]
do
	prgs[${#prgs[@]}]="$1"
	shift
done

shift
args=("$@")

for((nw = 1; nw <= $UPTO; nw <<= 2))
do
	exectimes=()

	for i in "${prgs[@]}"
	do
		echo "warming with: ./$i $nw ${args[@]}" 1>&2

		{ time ./$i $nw ${args[@]}; } > /dev/null

		echo "testing with: ./$i $nw ${args[@]}" 1>&2
			
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
