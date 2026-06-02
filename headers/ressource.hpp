#ifndef RESSOURCE_H 
#define RESSOURCE_H

/**
 * Client interface to interact with a remote ressource managed by a server. To
 * work propperly, any instance of this class must first call ressource_init and
 * be in a network with a valid server managing the ressource. 
 *
 * It's important to mention that the present class IS NOT thread-safe. Should
 * two threads from the host attempt to lock the ressource, the second call will
 * be blocked forever
 */
class ressource_t{
  int socket_fd=-1; 

  public:
    /**
     * Attempts to connect to the server managing the ressource
     * @param cp IP of the server responsible for managing the ressource
     * @return 0 if success, -1 if host fails to reach the server
     */
    int ressource_init(const char *cp);

    /**
     * Blocks until the host receives the ressource from the server
     * @return 0 if success, -1 if host fails to either send the LOCK message to
     *           or receive the GRANT message from the server.
     */
    int ressource_lock();


    /**
     * Sends the server a message releasing the ressource 
     * @return 0 if success, -1 if host fails to send the RELEASE message to the
     *         server
     */
    int ressource_release();

    /**
     * Closes the socket connecting the host to the server
     * @return the result of calling the close(socket_fd);
     */
    int ressource_clean_up();
};

#endif
