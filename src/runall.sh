#!/bin/sh

set -e

declare -i upto=512
declare -i szmatmul=2	# will be multiplied by 2^9
declare -i italloc=4	# will be multiplied by 2^20
declare -i itexchg=64	# will be multiplied by 2^10
declare -i hplen=2048	# will be multiplied by 2^10

fifo="/tmp/tp-bench-fifo.$$"
base="$(dirname $0)"

function formatout() \
{
	while test "$1" != ''
	do
		printf "% 10s" "$1"
		shift
	done
	echo
}

function timetosec() \
{ 
	awk '/^real.*/ {s=$2; sub("m","*60+",s); sub("s","\n",s); print s}' | bc
}

function runsingle() \
(
	set -m
	trap "kill -INT %1" INT

	local -i i=$1
	local -i nw=$2

	local cmd="${commands[$i]}"
	local args="${arguments[$i]}"

#	echo ./$cmd $nw $args 1>&2
	
	# give some time for plumbing to settle
	( sleep 1s;
		eval time ./$cmd $nw $args ) 1>/dev/null 2>"$fifo" &

	t=$(cat "$fifo" | timetosec)
	wait $! || t="FAIL"
	echo $t
)

function testone() \
{
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
		info=($nw);
		for((i = 0; i < ${#commands[@]}; i += 1))
		do
			info[${#info[@]}]=$(runsingle $i $nw 2>/dev/null)
		done; 

		formatout "${info[@]}"
	done
}

cd "$base"
mkfifo "$fifo"
trap "rm '$fifo'" EXIT

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
{
	printf "%s" "$(grep 'model name' /proc/cpuinfo | uniq -c \
		| sed -ne 's/.*\([0-9]\+\).*model name.*: \(.*\)/\1 on \2/g p')"
}

function showhuge() \
{
	printf "%s %s" "$(ls /sys/kernel/mm/hugepages 2>/dev/null)" \
		"$(cat /sys/kernel/mm/*transparent_hugepage/enabled)"
}

echo -e "testing with:\n" \
	"\tup to: $upto jobs\n" \
	"\tmatrix size: $szmatmul * 512\n" \
	"\talloc iterations: $italloc * 1024 * 1024\n" \
	"\texchange iterations: $itexchg * 1024\n" \
	"\tassumed huge page length: $hplen * 1024\n" \
	"\taffinity: $(taskset -cp $$)\n" \
	"\tcores: $(showcores)\n" \
	"\tsystem: $(uname -ro)\n" \
	"\thuge pages: $(showhuge)\n" \
	"\tlibc: $(/lib/libc.* | head -n 1)" 

function emitmatmul() \
{
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
}

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
	declare -i it=$(($italloc * 1024 * 1024))
	echo -e "\nallocation. iterations: $it"
	testone "T at $it" "P ap $it"
fi

if test $itexchg -gt 0
then
	declare -i it=$(($itexchg * 1024))
	echo -e "\nexchanges. iterations: $it"
	testone "T et $it" "P ep $it"
fi
