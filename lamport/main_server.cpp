#include "timed_messaging.h"
#include "logical_clock.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#define MAX 80 
#define PORT 8080 

int main(void){
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if(server_socket == -1){
    printf("Failed to Create Socket\n");
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

  // Wait for client to connect, then open a socket
  int client_socket = accept(server_socket, NULL, NULL);

  logical_clock_t* clock;
  new_clock(&clock);

  char message[] = "Hello, World!";

  // Send message to the client
  timed_send(client_socket, message, strlen(message), 0, clock);

  uint32_t time;
  get_time(clock,&time);
  printf("time %u\n",time);

  // Close the client socket
  close(client_socket);

  return 0;
}
