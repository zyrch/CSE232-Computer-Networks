
all: clean server client

server: server.c 
		g++ server.c -o server_ -lpthread

client: client.c
		g++ client.c -o client_ -lpthread

clean:
		rm -f server_ client_ server/* client/*

clean_resp:
		rm response_times/*

.PHONY: all server client clean
