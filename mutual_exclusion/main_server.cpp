#include "request_message.hpp"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAX 80 
#define PORT 8080 
#define NUM_NEIGHBOURS 3

request_queue_t<std::pair<request_type_e, int>> requests;
request_queue_t<int> requesters;
request_queue_t<int> releasers;
sem_t released;

void* answer_requests_th(void* data){
  while(true){
    requests.req_wait();
    std::pair<request_type_e, int> req = requests.req_pop();

    switch(req.first){
      case REQUEST_TYPE_LOCK:
        printf("Received a lock request from %d\n",req.second);
        requesters.req_push(req.second);
        break;

      case REQUEST_TYPE_RELEASE:
        printf("Received a release request from %d\n", req.second);
        releasers.req_push(req.second);
        break;

      default: 
        printf("The server can't process this type of request\n");
    }
  }
}


void* manage_ressource_th(void* data){
  while(true){
    requesters.req_wait();
    int requester = requesters.req_pop();

    request_type_e grant = REQUEST_TYPE_GRANT;
    printf("Granted Ressource to %d\n", requester);
    send(requester,&grant,sizeof(request_type_e),0);

    while(releasers.req_wait()){
      int releaser = releasers.req_pop();
      if(requester == releaser){
        break;
      }
      printf("Attempted to release ressource without being the owner\n");
    }
    printf("Ressource Released by %d\n",requester);
  }
}

void* reader_th(void* socket_fd_ptr){
  printf("Readers Thread Created\n");
  int socket_fd = *((int*) socket_fd_ptr);
  while(true){
    request_type_e msg;
    printf("Waiting on a Message\n");
    ssize_t recv_msg_size = recv(socket_fd,&msg,sizeof(request_type_e),0);

    if(recv_msg_size == -1){
      printf("Error reading from connection %d\n", socket_fd);
      break;
    }
    if(recv_msg_size ==0)
      break;

    printf("Received a Message\n");

    requests.req_push(std::pair<request_type_e,int>{msg,socket_fd});
  }

  // if connection is closed, it automatically
  // releases the ressource in case it had been locked
  releasers.req_push(
    socket_fd
  );

  close(socket_fd);
  printf("The connection %d has been closed\n", socket_fd);
  return nullptr;
}

int main(void){
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == -1){
    printf("Failed to Create Socket");
    return -1;
  }
  printf("Socket Created\n");

  struct sockaddr_in address;
  memset(&address, '\0', sizeof(struct sockaddr_in));

  address.sin_family = AF_INET;
  address.sin_port = htons(8080); // Port 8080
  address.sin_addr.s_addr = INADDR_ANY;

  // Bind server_socket to address
  int bind_res = bind(server_socket, (struct sockaddr *) &address, sizeof(address));

  if(bind_res==-1){
    printf("Failed to Bind");
    return -1;
  }

  printf("Bound Socket\n");

  // Listen for clients and allow the accept function to be used
  // Allow 4 clients to be queued while the server processes
  listen(server_socket, 4);
  printf("Listening to connections\n");

  sem_init(&released,0,1);
  requests.req_init();
  requesters.req_init();
  releasers.req_init();

  pthread_t request_handler;
  pthread_t resource_manager;
  pthread_create(&request_handler,nullptr,&answer_requests_th,nullptr);
  pthread_create(&resource_manager,nullptr,&manage_ressource_th,nullptr);

  //TODO: create one connection by
  //   // close(client_socket);
  while(true){
    int client_socket = accept(server_socket, NULL, NULL);
    pthread_t reader;
    pthread_create(&reader,nullptr, &reader_th,&client_socket);
  }

  return 0;
}
