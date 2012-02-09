#!/bin/sh

set -e

upto=512 
szmatmul=2 # will be multiplied by 2^9
italloc=4 # will be multiplied by 2^20
itexchg=64 # will be multiplied by 2^10

fifo="/tmp/tp-test.$$"
base="$(dirname $0)"

function formatout() \
(
	while test "$1" != ''
	do
		printf "% 12s" "$1"
		shift
	done
	echo
)

function testone() \
(
	header=(N)
	commands=()
	arguments=()

	for i in "$@"
	do
		info=($i)

		header[${#header[@]}]="${info[0]}"
		commands[${#commands[@]}]="${info[1]}"
		
		argpos=${#arguments[@]}
		for((j = 2; j < ${#info[@]}; j += 1))
		do
			arguments[$argpos]+=" '${info[$j]}'"
		done
	done

	formatout "${header[@]}"

	for((nw = 1; nw <= 2; nw <<= 1))
	do
		info=($nw)

		for((i = 0; i < ${#commands[@]}; i += 1))
		do
			echo ./"${commands[$i]}" $nw ${arguments[$i]}

			(time ./"${commands[$i]}" $nw ${arguments[$i]}) \
				2>"$fifo" &

			t=$(cat "$fifo" | awk '/^real.*/ {print $2}')

			wait $! || t="FAIL"

			info[${#info[@]}]="$t"
		done

		formatout "${info[@]}"
	done

	exit

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

			{ time ./$i $nw ${args[@]}; } &> /dev/null

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
)

mkfifo "$fifo"
cd "$base"

while test "$1" != '';
do
	case "$1" in

	'-u') shift; if test "$1" -gt 0; then upto=$1; fi;;
	'-m') shift; if test "$1" -gt 0; then szmatmul=$1; fi;;
	'-a') shift; if test "$1" -gt 0; then italloc=$1; fi;;
	'-e') shift; if test "$1" -gt 0; then itexchg=$1; fi;;

	esac

	shift
done

echo -e "testing with:\n" \
	"\tup to: $upto jobs\n" \
	"\tmatrix size: $szmatmul * 512\n" \
	"\talloc iterations: $italloc * 1024 * 1024\n" \
	"\texchange iterations: $itexchg * 1024\n" \
	"\taffinity: $(taskset -p $$)"

sz=$((szmatmul * 512))
echo -e "\nrow-stored matrix multiplication. size: $sz"

testone "T mnt $sz" "P mnp $sz" "P-FS mnpf $sz" \
	"HP.P mnp $sz $((2 << 20))" "HP.P-FS mnpf $sz $((2 << 20))"

it=$(($italloc * 1024 * 1024))
echo -e "\nallocation. iterations: $it"

rm "$fifo"
