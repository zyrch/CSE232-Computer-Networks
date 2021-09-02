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
#define NUM_CLIENTS 2

char N[] = "10";
char FILENAME[] = "processes_info_client";

struct process {
  int pid;
  char pname[1024];
  unsigned long int time;
};


// creates and binds socket to specified port
void * start_connection(void *arg);
void recv_file(int connectionfd, char *fileName);

int main() {

  pthread_t thread[NUM_CLIENTS];
  for (int i = 0; i < NUM_CLIENTS; ++i) {

    int thread_config = 8080;
    int res = pthread_create(&thread[i], NULL, start_connection, (void *)&thread_config);

    if (res < 0) {
      perror("pthread_create");
      return -1;
    }
  }
  
  for (int i = 0; i < NUM_CLIENTS; ++i) {
    pthread_join(thread[i], NULL);
  }
}

void recv_file(int connectionfd, char *fileName) {
  
  char buf[1200];

  char fileNameFull[256];
  sprintf(fileNameFull, "./%s_%d", fileName, connectionfd);
  
  FILE *file = fopen(fileNameFull, "w");

  while(1) {
    int bytes_received = recv(connectionfd, buf, sizeof(buf), 0);
    if (bytes_received < 0) {
      perror("send fil1e");
      return;
    }
    if (bytes_received == 0) {
      printf("Connection Closed");
      return;
    }
    fprintf(file, "%s", buf);

    // $ denotes end of file
    if (buf[strlen(buf) - 1] == '$') {
      break;
    }
  }

  fclose(file);
}

void send_top_process(int connectionfd, char *fileName) {

  char fileNameFull[256];
  sprintf(fileNameFull, "./%s_%d", fileName, connectionfd);
  
  FILE *file = fopen(fileNameFull, "r");

  process p;

  // file line of is the largest consuming process
  char top_process[1200];
  fscanf(file, "%d %s %lu\n", &p.pid, p.pname, &p.time); 
  sprintf(top_process, "%d %s %lu", p.pid, p.pname, p.time);

  int bytes_sent = send(connectionfd, top_process, sizeof(top_process), 0);
  if (bytes_sent < 0) {
    perror("send");
    exit(-1);
  }
  if (bytes_sent == 0) {
    printf("connected closed");
    exit(-1);
  }
  fclose(file);

}



void* start_connection(void *arg) {

  // side effects - Output to screen (IO)

  printf("Starting server...\n");

  int port = *((int *)arg);
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
  if (sockfd < 0) {
    perror("socket");
    exit(-1);
  }

  int yes = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    perror("setsockopt");
    exit(-1);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_port = htons(port);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;

  socklen_t server_addr_size = sizeof(server_addr);

  if (connect(sockfd, (struct sockaddr*)&server_addr, server_addr_size) < 0) {
    perror("connect");
    exit(-1);
  }

  printf("Connection establised on port %d\n", PORT);

  int bytes_sent = send(sockfd, N, sizeof(N), 0);

  if (bytes_sent < 0) {
    perror("send");
    exit(-1);
  }
  if (bytes_sent == 0) {
    printf("connected closed");
    exit(-1);
  }

  recv_file(sockfd, FILENAME);
  send_top_process(sockfd, FILENAME);

  close(sockfd);

  return ((void *)0);
}
