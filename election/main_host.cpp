#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define RETRY_TIME 1
#define MAX_NETWORK_SIZE 30
#define ERR_GUARD(x,msg) if((x)){printf(msg); exit(-1);}

typedef enum {
  ELECTION_MSG_ELECTION,
  ELECTION_MSG_OK,
  ELECTION_MSG_WINNER
}election_msg_e;

sem_t in_election; 
sem_t lost_election; 

int socket_fds[MAX_NETWORK_SIZE];
char** ip_addrs;
int host_id;
int num_neighbours;
int leader = -1; 

void call_election(){
  for(int i=host_id+1; i<num_neighbours; i++){
    int fd = socket_fds[i];

    //skip if neighbour's connection isn't alive
    if(fd == -1)
      continue;
  
    election_msg_e msg = ELECTION_MSG_ELECTION;
    send(fd,&msg,sizeof(election_msg_e),0);
  }
  sem_post(&in_election);

  const struct timespec election_response_time = {.tv_sec = 2, .tv_nsec=0};
  int wait_res = sem_timedwait(&lost_election,&election_response_time);
  if(wait_res==-1 && errno == ETIMEDOUT){
    sem_wait(&in_election);
    leader = host_id;

    for(int i=0; i<num_neighbours; i++){
      if(host_id == i)
        continue;

      int fd = socket_fds[i];
      //skip if neighbour's connection isn't alive
      if(fd == -1)
        continue;
    
      election_msg_e msg = ELECTION_MSG_WINNER;
      send(fd,&msg,sizeof(election_msg_e),0);
    }

  }
}

void* attempt_conn_th(void* neighbour_id_ptr);

void* connection_th(void* neighbour_id_ptr){
  int neighbour_id = *((int*)neighbour_id_ptr);
  free(neighbour_id_ptr);
  int fd = socket_fds[neighbour_id];
  while(true){
    election_msg_e msg;

    ssize_t msg_size = recv(fd,&msg,sizeof(election_msg_e),0);
    if(msg_size == -1)
      break;

    switch(msg){
      case ELECTION_MSG_ELECTION:
        if(neighbour_id<host_id){
          election_msg_e resp = ELECTION_MSG_OK;
          send(fd,&resp,sizeof(election_msg_e),0);
          call_election();
        }
        else{
          printf("Unexpected election from bigger host\n");
        }
        break;

      case ELECTION_MSG_OK:
        if(sem_trywait(&in_election) == 0)
          sem_post(&lost_election);
        break;

      case ELECTION_MSG_WINNER:
        leader = neighbour_id;
        break;

      default: 
        printf("This type of message is not treated\n");
        break;
    }

  }

  pthread_t tid;
  int *id = (int*)malloc(sizeof(int));
  int pthread_create_res = pthread_create(&tid,nullptr,&attempt_conn_th,id);

  return nullptr;
}

void* attempt_conn_th(void* neighbour_id_ptr){
  int neighbour_id = *((int*)neighbour_id_ptr);
  free(neighbour_id_ptr);

  ERR_GUARD(neighbour_id == host_id, "Attempted to connect to current host\n");

  if(neighbour_id > host_id){
    socket_fds[neighbour_id] = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    memset(&address, '\0', sizeof(struct sockaddr_in));

    address.sin_family = AF_INET;
    address.sin_port = htons(8080); // Port 8080
    //TODO: is this correct
    //address.sin_addr.s_addr = INADDR_ANY;
    inet_aton(ip_addrs[neighbour_id], (struct in_addr *) &(address.sin_addr.s_addr));

    int bind_res = bind(socket_fds[neighbour_id], (struct sockaddr *) &address, sizeof(address));
    listen(socket_fds[neighbour_id], 1);
    accept(socket_fds[neighbour_id],NULL,NULL);
  }

  else{
    socket_fds[neighbour_id] = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);

    inet_aton(ip_addrs[neighbour_id], (struct in_addr *) &(address.sin_addr.s_addr));

    // Establish a connection to address on client_socket
    int connect_res = -1;
    while(connect_res == -1){
      connect_res = connect(socket_fds[neighbour_id], (struct sockaddr *) &address, sizeof(address));
      sleep(RETRY_TIME);
    }
  }

  pthread_t tid;
  int *id = (int*)malloc(sizeof(int));
  int pthread_create_res = pthread_create(&tid,nullptr,&connection_th,id);

  return nullptr;
}


int main(int argc, char** argv){
  ERR_GUARD(argc < 3, "Insufficient command line arguments provided\n");

  num_neighbours = atoi(argv[1]);
  ERR_GUARD(num_neighbours>MAX_NETWORK_SIZE, "Insufficient command line arguments provided\n");

  host_id = atoi(argv[2]);
  ERR_GUARD(host_id >= num_neighbours, "Invalid host_id for network size\n");
  ERR_GUARD(argc < 3+num_neighbours, "Insufficient IP address for network size\n");

  memset(socket_fds,0,MAX_NETWORK_SIZE*sizeof(int));
  ip_addrs = &(argv[3]);

  for(int i=0;i<num_neighbours; i++){
    if(i==host_id)
      continue;

    pthread_t tid;
    int *id = (int*)malloc(sizeof(int));
    int pthread_create_res = pthread_create(&tid,nullptr,&attempt_conn_th,id);
  }

  leader = num_neighbours -1; // assumes the highest id in network is the leader
  
  // loop print who is the leader or call election 
  while(true){
    if(socket_fds[leader]==-1){
      printf("Leader unreachable, calling election\n");
      call_election();
    }

    printf("%d\n",leader);
    sleep(2); // Make the output of the hosts more interpretable 
  }

  return 0;
}
