set -e

[ -p /tmp/test-fifo ] || mkfifo /tmp/test-fifo

(time sleep 5s; true) >/tmp/test-fifo 2>&1 &
echo HI: $!

x="$(cat /tmp/test-fifo)"

wait $! && echo X: "$x" :Y || echo FAIL
