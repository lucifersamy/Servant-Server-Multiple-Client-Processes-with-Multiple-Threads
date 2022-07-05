CC = gcc
CFLAGS = -Wall
LIBS = -pthread -lrt
OBJ_SERVER = server.o
OBJ_SERVANT =servant.o
OBJ_CLIENT = client.o
LINKEDLIST = LinkedList.c
QUEUE = Queue.c
NET = network.c

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

all: server servant client

servant: $(OBJ_SERVANT)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(LINKEDLIST) $(NET)

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(NET)

server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(QUEUE) $(NET)

clean:
	rm -f *.o server servant client