#include <time.h>
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

sem_t start_election;
sem_t in_election; 
sem_t lost_election; 

int socket_fds[MAX_NETWORK_SIZE];
sem_t socket_sems[MAX_NETWORK_SIZE];

char** ip_addrs;
int host_id;
int num_neighbours;
int leader = -1; 

void call_election(){
  if(sem_trywait(&start_election) == -1){
    printf("already in election\n");
    return;
  }
  printf("starting an election\n");
  sem_post(&in_election);
  for(int i=host_id+1; i<num_neighbours; i++){

    sem_wait(&socket_sems[i]);
    int fd = socket_fds[i];
    sem_post(&socket_sems[i]);

    //skip if neighbour's connection isn't alive
    if(fd == -1)
      continue;
  
    sem_wait(&socket_sems[i]);
    election_msg_e msg = ELECTION_MSG_ELECTION;
    send(fd,&msg,sizeof(election_msg_e),0);
    sem_post(&socket_sems[i]);
  }

  struct timespec election_response_time;
  ERR_GUARD(clock_gettime(CLOCK_REALTIME, &election_response_time), "failed to get current time");
  election_response_time.tv_sec += 2;
  int wait_res = sem_timedwait(&lost_election,&election_response_time);
  if(wait_res==-1 && errno == ETIMEDOUT){
    sem_wait(&in_election);
    leader = host_id;
    printf("Became leader\n");

    for(int i=0; i<num_neighbours; i++){
      if(host_id == i)
        continue;

      sem_wait(&socket_sems[i]);
      int fd = socket_fds[i];
      sem_post(&socket_sems[i]);
      //skip if neighbour's connection isn't alive
      if(fd == -1)
        continue;
    
      sem_wait(&socket_sems[i]);
      election_msg_e msg = ELECTION_MSG_WINNER;
      send(fd,&msg,sizeof(election_msg_e),0);
      sem_post(&socket_sems[i]);
    }

  }
  sem_post(&start_election);
}

void* attempt_conn_th(void* neighbour_id_ptr);

void* connection_th(void* neighbour_id_ptr){
  int neighbour_id = *((int*)neighbour_id_ptr);
  free(neighbour_id_ptr);
  int fd = socket_fds[neighbour_id];
  printf("Start of neighbour %d reader thread\n", neighbour_id);
  while(true){
    election_msg_e msg;

    printf("waiting for a message from neighbour %d\n", neighbour_id);
    ssize_t msg_size = recv(fd,&msg,sizeof(election_msg_e),0);

    if(msg_size == 0){
      break;
    }
    else if(msg_size == -1){
      printf("Failed receival of message\n");
      break;
    }

    printf("received a message from neighbour %d of size %ld\n", neighbour_id, msg_size);

    switch(msg){
      case ELECTION_MSG_ELECTION:
        if(neighbour_id<host_id){
          election_msg_e resp = ELECTION_MSG_OK;
          sem_wait(&socket_sems[neighbour_id]);
          send(fd,&resp,sizeof(election_msg_e),0);
          sem_post(&socket_sems[neighbour_id]);
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
        if(leader < host_id)
          call_election();

        break;

      default: 
        printf("This type of message is not treated\n");
        break;
    }

  }

  printf("lost connection with %d\n", neighbour_id);
  sem_wait(&socket_sems[neighbour_id]);
  close(socket_fds[neighbour_id]);
  socket_fds[neighbour_id] = -1;
  sem_post(&socket_sems[neighbour_id]);

  if(neighbour_id < host_id){
    printf("creating attempt connection thread\n");
    pthread_t tid;
    int *id = (int*)malloc(sizeof(int));
    *id = neighbour_id;
    int pthread_create_res = pthread_create(&tid,nullptr,&attempt_conn_th,id);
  }


  if(leader == neighbour_id)
    call_election();
  return nullptr;
}

void connection_established(int neighbour_id){
  printf("established connection with %d\n", neighbour_id);
  pthread_t tid;
  int *id = (int*)malloc(sizeof(int));
  *id = neighbour_id;
  int pthread_create_res = pthread_create(&tid,nullptr,&connection_th,id);
}

void* attempt_conn_th(void* neighbour_id_ptr){
  int neighbour_id = *((int*)neighbour_id_ptr);
  free(neighbour_id_ptr);

  printf("attempted to connect to %d\n", neighbour_id);

  ERR_GUARD(neighbour_id == host_id, "Attempted to connect to current host\n");

  ERR_GUARD(neighbour_id > host_id, "Attempted to connect when it should be server");

  sem_wait(&socket_sems[neighbour_id]);
  socket_fds[neighbour_id] = socket(AF_INET, SOCK_STREAM, 0);
  sem_post(&socket_sems[neighbour_id]);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(8080);

  inet_aton(ip_addrs[neighbour_id], &(address.sin_addr));

  // Establish a connection to address on client_socket
  printf("checking if server in %d is online on ip %s\n", neighbour_id, ip_addrs[neighbour_id]);

  while(true){
    int connect_res = connect(
      socket_fds[neighbour_id],
      (struct sockaddr *)&address,
      sizeof(address));
    if(connect_res == 0)
      break;
    else if(connect_res == ECONNREFUSED)
      sleep(1); // wait some time in case server was not up yet
    else
      ERR_GUARD(0==1, "Failed to connect to neighbour\n");
    
  }

  connection_established(neighbour_id);
  call_election();
  return nullptr;
}

int get_neighbour_id(char* ip){
  for(int i=0; i<num_neighbours; i++){
    if(strcmp(ip_addrs[i],ip)==0)
      return i;
  }
  return -1;
}

void* server_th(void* data){
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in address;
  memset(&address, '\0', sizeof(struct sockaddr_in));

  address.sin_family = AF_INET;
  address.sin_port = htons(8080); // Port 8080
  address.sin_addr.s_addr = INADDR_ANY;
  int bind_res = bind(server_socket, (struct sockaddr *) &address, sizeof(address));
  listen(server_socket, MAX_NETWORK_SIZE);

  while(true){
    printf("waiting on connection\n");
    int neighbour_socket  = accept(server_socket,NULL,NULL);
    struct sockaddr_in neighbour_addr; 
    socklen_t addr_len = sizeof(struct sockaddr_in);

    ERR_GUARD(
      getpeername(neighbour_socket, (struct sockaddr *) &neighbour_addr, &addr_len)==-1,
      "Failed to resolve Remote Host Name"
    ); 

    char neighbour_ip[20];
    strcpy(neighbour_ip, inet_ntoa(neighbour_addr.sin_addr));

    int neighbour_id = get_neighbour_id(neighbour_ip);
    if(neighbour_id == -1){
      printf("unknown neighbour attempted to connect, ignored it\n");
      continue;
    }

    ERR_GUARD(neighbour_id == host_id, "Connected with neighbour with same id as host\n");
    
    sem_wait(&socket_sems[neighbour_id]);
    socket_fds[neighbour_id] = neighbour_socket;
    sem_post(&socket_sems[neighbour_id]);

    connection_established(neighbour_id);
  }
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
  for(int i=0; i<num_neighbours; i++)
    socket_fds[i] =-1;

  sem_init(&start_election,0,1);
  sem_init(&in_election,0,0);
  sem_init(&lost_election,0,0);
  for(int i=0;i<num_neighbours;i++)
    sem_init(&socket_sems[i],0,1);

  ip_addrs = &(argv[3]);

  printf("host with IP %s", ip_addrs[host_id]);
    
  pthread_t tid_server;
  int pthread_create_res = pthread_create(&tid_server,nullptr,&server_th,nullptr);

  for(int i=0;i<host_id; i++){
    printf("created thread for %d\n", i);
    pthread_t tid;
    int *id = (int*)malloc(sizeof(int));
    *id = i;
    int pthread_create_res = pthread_create(&tid,nullptr,&attempt_conn_th,id);
  }

  leader =  -1; // assumes the highest id in network is the leader
  
  // loop print who is the leader or call election 
  while(true){
    if(leader==-1){
      printf("No leader\n");
      call_election();
    }

    printf("%d\n",leader);
    sleep(2); // Make the output of the hosts more interpretable 
  }

  return 0;
}
