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

#define PORT 8080
#define BACKLOG 32

#include <vector>
#include <algorithm>

char FILENAME[] = "processes_info";

struct process {
  int pid;
  char pname[1024];
  unsigned long int utime, stime;

  bool operator <(const process& other) {
    return (utime + stime) > (other.utime + other.stime);
  }
};

// creates and binds socket to specified port
int start_listening(int port);
void* serve_connection(void *arg);
void write_to_buffer(char *buffer, char *str);
void write_processes_to_file(const std::vector<process> &processes, char *fileName, int connectionfd);
void send_file(int connectionfd, char *fileName);

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

    int *thread_config = (int *)malloc(sizeof(int));
    *thread_config = connectionfd;

    int res = pthread_create(&thread, NULL, serve_connection, (void *)thread_config);

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

  char buf[1024];
  int message_length = recv(connectionfd, buf, 1024, 0);
  if (message_length < 0) {
    perror("recv");
    exit(-1);
  }
  if (message_length == 0) {
    printf("connection closed");
    pthread_exit(NULL);
  }
  int N = atoi(buf);

  struct dirent *de;
  DIR *dir_proc = opendir("/proc");
  if (dir_proc == NULL) {
    perror("opendir");
    exit(-1);
  }

  std::vector<process> processes;

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
    FILE *file;

    file = fopen(fileName, "r");
    
    if (file == NULL) {
      perror("file");
      continue;
    }
    process p;

    fscanf(file, "%d %s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu", &p.pid, p.pname, &p.stime, &p.utime); 

    processes.push_back(p);

    fclose(file);
  }

  closedir(dir_proc);


  std::sort(processes.begin(), processes.end());
  while(processes.size() > N) {
    processes.pop_back();
  }

  write_processes_to_file(processes, FILENAME, connectionfd);
  send_file(connectionfd, FILENAME);

  message_length = recv(connectionfd, buf, 1024, 0);
  if (message_length < 0) {
    perror("recv");
    exit(-1);
  }
  if (message_length == 0) {
    printf("connection closed");
    pthread_exit(NULL);
  }

  printf("Process info sent by client %s\n", buf);
  fflush(stdout);

  free (arg);

  return (void *)0;

}

void write_processes_to_file(const std::vector<process> &processes, char *fileName, int connectionfd) {
  char buffer[8192 * 4];
  for (const process &p : processes) {
    char info[1200];
    sprintf(info, "%d %s %lu", p.pid, p.pname, p.stime + p.utime);
    write_to_buffer(buffer, info);
  }

  char end[] = "$";
  sprintf(&buffer[strlen(buffer)], "%s", end);

  char fileNameFull[256];
  sprintf(fileNameFull, "./server/%s_%d", fileName, connectionfd);
  

  FILE *file = fopen(fileNameFull, "w");
  fprintf(file,"%s", buffer); 

  fclose(file);
}

void send_file(int connectionfd, char *fileName) {
  
  char buf[1200];

  char fileNameFull[256];
  sprintf(fileNameFull, "./server/%s_%d", fileName, connectionfd);
  
  FILE *file = fopen(fileNameFull, "r");

  while(fgets(buf, sizeof(buf), file) > 0) {
    int len = strlen(buf);
    int bytes_sent = send(connectionfd, buf, sizeof(buf), 0);
    if (bytes_sent < 0) {
      perror("send file");
      return;
    }
    if (bytes_sent == 0) {
      printf("Connection Closed");
      return;
    }
  }

  fclose(file);
}

void write_to_buffer(char *buffer, char *str) {
  sprintf(&buffer[strlen(buffer)], "%s\n", str);
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

