
all: clean server client

server: server.c 
		g++ server.c -o server_ -lpthread

client: client.c
		g++ client.c -o client_ -lpthread

run:
		./server_ & ./client_ && fg

clean:
		rm -f server_ client_ processes_info*

.PHONY: all server client clean
