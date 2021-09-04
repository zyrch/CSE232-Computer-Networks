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
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>

#define PORT 8080

int NUM_CLIENTS = 1;

char N[] = "30";
char FILENAME[] = "processes_info_client";

struct process {
  int pid;
  char pname[1024];
  unsigned long int time;
  unsigned long int utime, stime;
};


// creates and binds socket to specified port
void * start_connection(void *arg);
void recv_file(int connectionfd, char *fileName, int id);

int main(int argc, char* argv[]) {

  if (argc > 1) {
    NUM_CLIENTS = atoi(argv[1]);
    if (NUM_CLIENTS == 0) {
      printf("Invalid parameter 'NUM_CLIENTS'\n");
      return 1;
    }
  } else {
    printf("Number of clients not provided, using the default value of 1 \n");
  }

  pthread_t thread[NUM_CLIENTS];
  int thread_config[NUM_CLIENTS];
  for (int i = 0; i < NUM_CLIENTS; ++i) {

    thread_config[i] = i + 1;
    int res = pthread_create(&thread[i], NULL, start_connection, (void *)&thread_config[i]);

    if (res < 0) {
      perror("pthread_create");
      return -1;
    }
  }
  
  for (int i = 0; i < NUM_CLIENTS; ++i) {
    pthread_join(thread[i], NULL);
  }
}

void recv_file(int connectionfd, char *fileName, int id) {
  
  char buf[1200];

  char fileNameFull[256];
  sprintf(fileNameFull, "./client/%s_%d", fileName, id);
  
  FILE *file = fopen(fileNameFull, "w");

  while(1) {
    int bytes_received = recv(connectionfd, buf, sizeof(buf), 0);
    if (bytes_received < 0) {
      perror("send file");
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

void send_top_process(int connectionfd) {

  struct dirent *de;
  DIR *dir_proc = opendir("/proc");
  if (dir_proc == NULL) {
    perror("opendir");
    exit(-1);
  }

  process top_process;

  char fileName[1024];
  while((de = readdir(dir_proc)) != NULL) {
    
    int is_pid = 1;
    for (char *ch = de->d_name; *ch; ++ch) {
      if (!isdigit(*ch)) {
        is_pid = 0;
      }
    }
    if (!is_pid) continue;

    sprintf(fileName, "/proc/%s/stat", de->d_name);

    int file = open(fileName, O_RDONLY);

    if (file < 0) {
      // perror("open");
      continue;
    }

    char fileBuf [8192 * 4];
    int res = read(file, fileBuf, 8192 * 4);

    if (res < 0) {
      perror("read");
    }


    process p;

    sscanf(fileBuf, "%d %s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu", &p.pid, p.pname, &p.stime, &p.utime); 

    if (p.stime + p.utime > top_process.stime + top_process.utime) {
      top_process = p;
    }

    close(file);
  }

  closedir(dir_proc);

  top_process.time = top_process.stime + top_process.utime;

  // file line of is the largest consuming process
  char top_process_buf[1200];
  sprintf(top_process_buf, "%d %s %lu", top_process.pid, top_process.pname, top_process.time);

  int bytes_sent = send(connectionfd, top_process_buf, sizeof(top_process_buf), 0);
  if (bytes_sent < 0) {
    perror("send");
    exit(-1);
  }
  if (bytes_sent == 0) {
    printf("connected closed");
    exit(-1);
  }

}



void* start_connection(void *arg) {

  // side effects - Output to screen (IO)

  int id = *((int *)arg);
  int port = PORT;
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

  recv_file(sockfd, FILENAME, id);
  send_top_process(sockfd);

  close(sockfd);

  return ((void *)0);
}
