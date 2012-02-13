#!/bin/sh

set -me

declare -i upto
declare -i szmatmul
declare -i italloc
declare -i itexchg
declare -i hplen

upto=512 
szmatmul=2	# will be multiplied by 2^9
italloc=4	# will be multiplied by 2^20
itexchg=64	# will be multiplied by 2^10
hplen=2048	# will be multiplied by 2^10 

fifo="./bench-fifo.$$"
base="$(dirname $0)"

function formatout() \
(
	while test "$1" != ''
	do
		echo -ne "\t$1"
		shift
	done
	echo
)

function testone() \
(
	set -me

	trap "echo && jobs && kill -INT %1 && echo interrupted" INT
	
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

			time ./$cmd $nw $args 2>"$fifo" >/dev/null &

			t=$(cat "$fifo" \
				| awk '/^real.*/ {print $2}' \
				| sed -ne 's/m/*60 + /g; s/s/\n/g p' \
				| bc)
		
			wait $! || t="FAIL"

			info[${#info[@]}]=$t
		done

		formatout "${info[@]}"
	done
)

cd "$base"
mkfifo "$fifo"
trap "echo -e '\nexiting'; rm '$fifo'" EXIT

while test "$1" != '';
do
	case "$1" in

	'-u') shift; upto="$1";;
	'-m') shift; szmatmul="$1";;
	'-a') shift; italloc="$1";;
	'-e') shift; itexchg="$1";;
	'-hp') shift; hplen="$1";;

	esac

	shift
done

function showcores() \
(
	printf "%s" "$(grep 'model name' /proc/cpuinfo | uniq -c \
		| sed -ne 's/.*\([0-9]\+\).*model name.*: \(.*\)/\1 on \2/g p')"
)

function showhuge() \
(
	printf "%s %s" "$(ls /sys/kernel/mm/hugepages 2>/dev/null)" \
		"$(cat /sys/kernel/mm/*transparent_hugepage/enabled)"
)

echo -e "testing with:\n" \
	"\tup to: $upto jobs\n" \
	"\tmatrix size: $szmatmul * 512\n" \
	"\talloc iterations: $italloc * 1024 * 1024\n" \
	"\texchange iterations: $itexchg * 1024\n" \
	"\taffinity: $(taskset -cp $$)\n" \
	"\tcores: $(showcores)\n" \
	"\tsystem: $(uname -ro)\n" \
	"\thuge pages: $(showhuge)\n" \
	"\tlibc: $(/lib/libc.* | head -n 1)" 

function emitmatmul() \
(
	local -A mcmds=( \
		[naive]='([T]=mnt [T-M]=mntm [P]=mnp [P-FS]=mnpf)' \
		[tile]='([T]=mtt [T-M]=mttm [P]=mtp [P-FS]=mtpf)' )

	local -A mcmd=${mcmds[$1]}

	tcmd='testone'

	for i in T T-M P P-FS
	do
		tcmd+=" '$i ${mcmd[$i]} $sz'"
	done

	if test $hplen -gt 0
	then
		hp=$(($hplen << 10))
		for i in T-M P P-FS
		do
			tcmd+=" 'HP.$i ${mcmd[$i]} $sz $hp'"
		done
	fi
	echo "$tcmd"
)

if test $szmatmul -gt 0
then
	sz=$((szmatmul * 512))

	echo -e "\nrow-stored matrix multiplication. size: $sz"
	eval $(emitmatmul naive)

	echo -e "\ntile-stored matrix multiplication. size: $sz"
	eval $(emitmatmul tile)
fi

if test $italloc -gt 0
then
	local -i it=$(($italloc * 1024 * 1024))
	echo -e "\nallocation. iterations: $it"
	testone "T at $it" "P ap $it"
fi

if $itexchg -gt 0
then
	local -i it=$(($itexchg * 1024))
	echo -e "\nexchanges. iterations: $it"
	testone "T et $it" "P ep $it"
fi
