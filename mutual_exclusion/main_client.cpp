#include "ressource.hpp"
#include <stdio.h>

int main(int argc, char* argv[]){
  if(argc <2){
    printf("A Server IP address was not specified\n");
    return -1;
  }

  printf("Attempting to connect to server at %s\n", argv[1]);

  ressource_t ressource;
  if(ressource.ressource_init(argv[1])){
    printf("Failed to Establish Connection\n");
    return -1;
  }

  if(ressource.ressource_lock() == -1){
    printf("Failed to Lock Ressource\n");
    return -1;
  }
  printf("Locked Ressource\n");

  if(ressource.ressource_release() == -1){
    printf("Failed to Release Ressource\n");
    return -1;
  }
  printf("Released Ressource\n");

  if(ressource.ressource_clean_up() == -1){
    printf("Failed to Clean Up\n");
    return -1;
  }
  return 0;
}
