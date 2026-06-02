#include "timed_messaging.h"
#include "logical_clock.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
  if(argc < 2){
    printf("Did not receive IP of server\n");
    return -1;
  }
  printf("Attempting to connect to host at %s\n", argv[1]);

  int client_socket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(8080);

  // Set address to your computer's local address
  inet_aton(argv[1], (struct in_addr *) &(address.sin_addr.s_addr));

  // Establish a connection to address on client_socket
  int connect_res = connect(client_socket, (struct sockaddr *) &address, sizeof(address));
  if(connect_res == -1) {
    printf("failed to establish connection");
  }

  char message[256];
  memset(message, 0, 256);

  logical_clock_t* clock;
  new_clock(&clock);

  // Receive max of 255 characters from server (null terminated)
  timed_recv(client_socket, message, 255, 0, clock);

  // Close the connection
  close(client_socket);

  printf("received message %s\n", message);
  uint32_t time;
  get_time(clock,&time);
  printf("received at time %u\n",time);
  return 0;
}
