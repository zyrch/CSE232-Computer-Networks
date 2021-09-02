
all: clean server client

server: server.c
		g++ server.c -o server -lpthread

client: client.c
		g++ client.c -o client -lpthread

clean:
		rm -f server client processes_info*
