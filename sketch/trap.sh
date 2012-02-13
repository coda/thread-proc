./tl &
echo "kill -INT $!"
trap "echo sigint && kill %1" EXIT
wait $!
trap - EXIT
