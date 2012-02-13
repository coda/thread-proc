p=$(set -m;
	{ trap 'echo INT' INT; sleep 120s; echo DONE; } >>log &
	echo $!)

echo $p here
sleep 1s

kill -INT -$p

echo KILLED
