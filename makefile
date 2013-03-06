SOURCE1 = server.c
SOURCE2 = client.c
OBJECTS1 = server.o
OBJECTS2 = client.o
OBJECTS3 = hash.o
OBJECTS4 = client_functions.o
OBJECTS5 = pollSwayer.o
OBJECTS6 = server_states.o
OBJECTS7 = worker_server.o
OUT1 = poller
OUT2 = client
OUT3 = pollSway

CC = gcc
FLAGS = -g3 -c

all:	$(OUT1)	$(OUT2) $(OUT3)

$(OUT1): $(OBJECTS1) $(OBJECTS3) $(OBJECTS6) $(OBJECTS7) poller.h
	$(CC) -g -o $(OUT1) $(OBJECTS1) $(OBJECTS3) $(OBJECTS6) $(OBJECTS7) -pthread

$(OUT2): $(OBJECTS2) $(OBJECTS4) poller.h
	$(CC) -g -o $(OUT2) $(OBJECTS2) $(OBJECTS4)  -pthread

$(OUT3): $(OBJECTS5) $(OBJECTS4) poller.h
	$(CC) -g -o $(OUT3) $(OBJECTS5) $(OBJECTS4)  -pthread

server.o: server.c poller.h
	$(CC) $(FLAGS) server.c

client.o: client.c poller.h
	$(CC) $(FLAGS) client.c

hash.o: hash.c poller.h
	$(CC) $(FLAGS) hash.c

client_functions.o: client_functions.c poller.h
	$(CC) $(FLAGS) client_functions.c

pollSwayer.o: pollSwayer.c poller.h
	$(CC) $(FLAGS) pollSwayer.c

server_states.o: server_states.c poller.h
	$(CC) $(FLAGS) server_states.c

worker_server.o: worker_server.c poller.h
	$(CC) $(FLAGS) worker_server.c

clean:
	rm -f $(OUT1) $(OUT2) $(OUT3) $(OBJECTS1) $(OBJECTS2) $(OBJECTS3) $(OBJECTS4) $(OBJECTS5) $(OBJECTS6) $(OBJECTS7) *~
