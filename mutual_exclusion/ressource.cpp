#include "request_message.hpp"
#include "request_message.cpp"
#include "ressource.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int ressource_t::ressource_init(const char *cp){
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(8080);

  // Set address to your computer's local address
  inet_aton(cp, (struct in_addr *) &(address.sin_addr.s_addr));

  // Establish a connection to address on client_socket
  int connect_res = connect(client_socket, (struct sockaddr *) &address, sizeof(address));
  if(connect_res == -1)
    return -1;

  socket_fd = client_socket;
  return 0;
}

int ressource_t::ressource_lock(){
  if(socket_fd == -1) 
    return -1;

  request_type_e lock = REQUEST_TYPE_LOCK;
  int send_res = send(socket_fd,&lock,sizeof(request_type_e),0);
  if(send_res == -1)
    return -1;

  request_type_e resp;
  ssize_t resp_size = recv(socket_fd,&resp,sizeof(request_type_e),0);

  if(resp_size == -1)
    return -1;

  if(resp!=REQUEST_TYPE_GRANT){
    return -1;
  }

  return 0;
}

int ressource_t::ressource_release(){
  if(socket_fd == -1) 
    return -1;

  request_type_e release = REQUEST_TYPE_RELEASE;
  int send_res = send(socket_fd,&release,sizeof(request_type_e),0);
  return send_res;
}

int ressource_t::ressource_clean_up(){
  int close_res = close(socket_fd);
  return close_res;
}
