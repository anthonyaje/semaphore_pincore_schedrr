all : rr-task set_scheduler 

rr-task.o: rr-task.cc
	g++ -c rr-task.cc -Wno-write-strings -std=c++11 -o rr-task.o

set_scheduler.o: set_scheduler.cc
	g++ -c set_scheduler.cc -Wno-write-strings -std=c++11 -o set_scheduler.o

rr-task: rr-task.o
	g++ rr-task.o -lpthread -o rr-task

set_scheduler: set_scheduler.o
	g++ set_scheduler.o -lpthread -o set_scheduler 

clean:
	rm -f *.o
	rm -f rr-task
	rm -f set_scheduler
