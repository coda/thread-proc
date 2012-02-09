#!/bin/sh

set -e

upto=512 
szmatmul=2 # will be multiplied by 2^9
italloc=4 # will be multiplied by 2^20
itexchg=64 # will be multiplied by 2^10

fifo="./bench-fifo.$$"
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

	for((nw = 1; nw <= $upto; nw <<= 1))
	do
		info=($nw)

		for((i = 0; i < ${#commands[@]}; i += 1))
		do
			cmd="${commands[$i]}"
			args="${arguments[$i]}"

			(eval time ./"$cmd" $nw $args) 2>"$fifo" >/dev/null &
			t=$(cat "$fifo" | awk '/^real.*/ {print $2}')

			wait $! || t="FAIL"

			info[${#info[@]}]="$t"
		done

		formatout "${info[@]}"
	done
)

cd "$base"
mkfifo "$fifo"

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
	"\taffinity: $(taskset -cp $$)\n" \
	"\tsystem: $(uname -ro)\n" \
	"\tlibc: $(/lib/libc.* | head -n 1)\n" \
	"\tcpu: $(grep 'model name' /proc/cpuinfo \
		| sed -ne 's/.*: \(.*\)/\1/g p' | uniq -c)"

hp=$((2<<20))
sz=$((szmatmul * 512))

echo -e "\nrow-stored matrix multiplication. size: $sz"
testone "T mnt $sz" "P mnp $sz" "P-FS mnpf $sz" \
	"HP.P mnp $sz $hp" "HP.P-FS mnpf $sz $hp"

echo -e "\ntile-stored matrix multiplication. size: $sz"
testone "T mtt $sz" "P mtp $sz" "P-FS mtpf $sz" \
	"HP.P mtp $sz $hp" "HP.P-FS mtpf $sz $hp"

it=$(($italloc * 1024 * 1024))
echo -e "\nallocation. iterations: $it"
testone "T at $it" "P ap $it"

it=$(($itexchg * 1024))
echo -e "\nexchanges. iterations: $it"
testone "T et $it" "P ep $it"

rm "$fifo"
