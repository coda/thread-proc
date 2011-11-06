set -e

niter=4
nworkersupto=32

[ "$1" == '' ] || nworkersupto="$1"
[ "$2" == '' ] || niter="$2"

for((nw=1; nw <= nworkersupto; nw <<= 1))
do
	ptime=$({ time ./pt $(($niter * 1024 * 1024)) $nw; } 2>&1 \
		| awk '/^real.*/ {print $2}')

	ttime=$({ time ./tt $(($niter * 1024 * 1024)) $nw; } 2>&1 \
		| awk '/^real.*/ {print $2}')

	printf $niter"M / %- 8u\tT: $ttime P: $ptime\n" $nw
done
