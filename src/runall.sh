#!/bin/sh

set -e

declare -i upto=512
declare -i szmatmul=2	# will be multiplied by 2^9
declare -i italloc=4	# will be multiplied by 2^20
declare -i itexchg=64	# will be multiplied by 2^10
declare -i hplen=2048	# will be multiplied by 2^10 by test code

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

	echo ./$cmd -n $nw $args 1>&2
	
	# give some time for plumbing to settle
	( sleep 1s;
		eval time ./$cmd -n $nw $args ) 1>/dev/null 2>"$fifo" &

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
	"\tassumed huge page length: $hplen"'KiB\n' \
	"\taffinity: $(taskset -cp $$)\n" \
	"\tcores: $(showcores)\n" \
	"\tsystem: $(uname -ro)\n" \
	"\thuge pages: $(showhuge)\n" \
	"\tlibc: $(/lib/libc.* | head -n 1)" 

function emitmatmul() \
{
	local -A mcmds
	mcmds[naive]="([T-I]=mnt [P-I]=mnp "
	mcmds[naive]+=" [T]='mnt -a G' [P]='mnp -a G' [P-FS]='mnpf -a G')"

	mcmds[tile]="([T-I]=mtt [P-I]=mtp "
	mcmds[tile]+=" [T]='mtt -a G' [P]='mtp -a G' [P-FS]='mtpf -a G')"

	local -A mcmd=${mcmds[$1]}

	tcmd='testone'

	for i in T-I P-I T P P-FS
	do
		tcmd+=" '$i ${mcmd[$i]} -s $sz'"
	done

	if test $hplen -gt 0
	then
		for i in P P-FS
		do
			tcmd+=" 'HP.$i ${mcmd[$i]} -s $sz -p $hplen'"
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
	testone "T at -s $it" "P ap -s $it"
fi

if test $itexchg -gt 0
then
	declare -i it=$(($itexchg * 1024))
	echo -e "\nexchanges. iterations: $it"
	testone "T et -s $it" "P ep -s $it"
fi
