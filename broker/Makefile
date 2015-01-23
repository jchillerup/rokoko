default:
	gcc -lnanomsg -o robroker robroker.c
	gcc -lnanomsg -o testpublish testpublish.c

connect:
	nanocat --sub --connect tcp://localhost:14040 -A

publish:
	nanocat --push --connect ipc:///tmp/rokoko-socket.ipc -D "Hejsa"
