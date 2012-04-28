#!/bin/sh

set -e

declare -i upto=512
declare -i szmatmul=2	# will be multiplied by 2^9
declare -i italloc=4	# will be multiplied by 2^20
declare -i itexchg=64	# will be multiplied by 2^10
declare -i hplen=2048	# will be multiplied by 2^10 by test code

# declare -A affiname=(['-a G']='Group' ['-a I']='Interleave' [' ']='None')
declare -A affiname=(['-a I']='Interleave')

declare log='/dev/null'

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

	local cmd="$1"
	local -i nw=$2

	# give some time for plumbing to settle
	( echo -e "./$cmd -n $nw"; sleep 1s;
		eval time "./$cmd" -n $nw; echo ) >> "$log" 2>"$fifo" &

	t=$(cat "$fifo" | tee -a /dev/stderr | timetosec)
	wait $! || t="FAIL"
	echo $t
)

function testone() \
{
	local header=(N)
	local commands=()

	for i in "$@"
	do
		info=($i)

		head="${info[0]}"
		header[${#header[@]}]="$head"
		commands[${#commands[@]}]="${i#$head }"
	done

	formatout "${header[@]}"

	for((nw = 1; nw <= $upto; nw <<= 1))
	do
		info=($nw);
		for i in "${commands[@]}"
		do
			info[${#info[@]}]=$(runsingle "$i" $nw 2>/dev/null)
		done; 

		formatout "${info[@]}"
	done
}

while test "$1" != '';
do
	case "$1" in

	'-u') shift; upto="$1";;
	'-m') shift; szmatmul="$1";;
	'-a') shift; italloc="$1";;
	'-e') shift; itexchg="$1";;
	'-p') shift; hplen="$1";;
	'-l') shift; log="$(readlink -f "$1")";;
	'-s') affiname=(['-a I']='Interleave');;

	esac

	shift
done

cd "$base"
mkfifo "$fifo"
trap "rm '$fifo'" EXIT

function showcores() \
{
	printf "%s" "$(grep 'model name' /proc/cpuinfo | uniq -c \
		| sed -ne 's/[^0-9]*\([0-9]\+\).*model name.*: \(.*\)/\1 on \2/g p')"
}

function showhuge() \
{
	printf "%s %s" "$(ls /sys/kernel/mm/hugepages 2>/dev/null)" \
		"$(cat /sys/kernel/mm/*transparent_hugepage/enabled)"
}

echo -n 'checking tools: '
bc 2>/dev/null <<<'' || { echo "bc isn't operational"; false; }
wk 2>/dev/null ''<<<'' || { echo "awk isn't operational"; false; }
echo 'OK'

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
	local -A mcmds=( \
		[naive]="([T]=mnt [T-M]=mntm [P]=mnp [P-FS]=mnpf)" \
		[tile]="([T]=mtt [T-M]=mttm [P]=mtp [P-FS]=mtpf)")

	local -A mcmd=${mcmds[$1]}
	affarg="$2"

	tcmd='testone'

#	for i in T T-M P P-FS
	for i in T P T-M; do
		tcmd+=" '$i ${mcmd[$i]} $affarg -s $sz'"
	done

	if test $hplen -gt 0
	then
#		for i in T-M P P-FS
		for i in P T-M; do
			tcmd+=" 'HP.$i ${mcmd[$i]} $affarg -s $sz -p $hplen'"
		done
	fi
	echo "$tcmd"
}

for aff in "${!affiname[@]}"
do
	echo -e "\ntest round with affinity: ${affiname[$aff]}"

	if test $szmatmul -gt 0
	then
		sz=$((szmatmul * 512))

		echo -e "\nrow-stored matrix multiplication. size: $sz"
		eval $(emitmatmul naive "$aff")

		echo -e "\ntile-stored matrix multiplication. size: $sz;"
		eval $(emitmatmul tile "$aff")
	fi

	if test $italloc -gt 0
	then
		declare -i it=$(($italloc * 1024 * 1024))
		echo -e "\nallocation. iterations: $it"
		testone "T at $aff -s $it" "P ap $aff -s $it"
	fi

	if test $itexchg -gt 0
	then
		declare -i it=$(($itexchg * 1024))
		echo -e "\nexchanges. iterations: $it"
		testone "T et $aff -s $it" "P ep $aff -s $it"
	fi
done
