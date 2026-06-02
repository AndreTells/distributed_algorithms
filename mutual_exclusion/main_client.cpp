#include "ressource.hpp"
#include "ressource.cpp"
#include <stdio.h>

int main(void){
  ressource_t ressource;
  if(ressource.ressource_init("127.0.0.1")){
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
