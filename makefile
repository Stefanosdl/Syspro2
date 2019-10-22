mirror_client: main.o functions.o inotifyFunctions.o
	gcc -g -Wall main.o functions.o inotifyFunctions.o -o mirror_client

main.o: main.c
	gcc -c -g main.c

functions.o: functions.c ./headers/functions.h
	gcc -c -g functions.c

inotifyFunctions.o: inotifyFunctions.c ./headers/inotifyFunctions.h
	gcc -c -g inotifyFunctions.c

clean:
	rm -rf *.o mirror_client
