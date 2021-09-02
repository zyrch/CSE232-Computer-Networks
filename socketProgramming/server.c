#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define PORT 8080
#define BACKLOG 32

// creates and binds socket to specified port
int start_listening(int port);
void* serve_connection(void *arg);

int main() {

  int sockfd = start_listening(PORT);
 
  while(1) {

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int connectionfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);

    if (connectionfd < 0) {
      perror("accept");
    }
    
    pthread_t thread;

    int thread_config = connectionfd;

    int res = pthread_create(&thread, NULL, serve_connection, (void *)&thread_config);
    printf("res ");

    if (res < 0) {
      perror("pthread_create");
      return -1;
    }

  }

  close(sockfd);

  return 0;
}

void* serve_connection(void *arg) {

  int connectionfd = *((int *)arg);
  printf("Server started");  

}

int start_listening(int port) {

  // side effects - Output to screen (IO)

  printf("Starting server...\n");

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
  if (sockfd < 0) {
    perror("socket");
    return -1;
  }

  int yes = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    perror("setsockopt");
    return -1;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_port = htons(port);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;

  socklen_t server_addr_size = sizeof(server_addr);

  if (bind(sockfd, (struct sockaddr*)&server_addr, server_addr_size) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    return -1;
  }

  printf("Listening on port %d\n", PORT);

  return sockfd;
}

