#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <csapp.h>
#include <server.h>

CLIENT_REGISTRY *client_registry;

void terminate(int status);
static void SIGHUP_handler(int status){
    int save_errno = errno;
    terminate(0);
    errno = save_errno;
}

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char* port_number;
    char* hostname;
    char optval;
    while(optind < argc) {
        if((optval = getopt(argc, argv, "p:o:q")) != -1) {
            if(strcmp(argv[1],"-p") != 0 && strcmp(argv[1],"-o") != 0 && strcmp(argv[1],"-q") != 0 ){
                fprintf(stderr, "Usage: %s [-p <Port>] [-h <hostname>] [-q]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
            }
            switch(optval) {
                case 'p':
                    port_number = optarg;
                    if(port_number == NULL){
                        fprintf(stderr, "%s\n", "Port number must be specified.");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'h':
                    hostname = optarg;
                    fprintf(stdout, "%s\n", hostname);
                    break;
                case 'q':
                    //set the variable to indicate run without prompting
                    break;
               case '?':
                  fprintf(stderr, "Usage: %s [-p <Port>] [-h <hostname>] [-q]\n", argv[0]);
                  exit(EXIT_FAILURE);
                  break;
            default:
                break;
            }
        }
        }


    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    Signal(SIGHUP,SIGHUP_handler);
    int* connfd;
    int listenFd = Open_listenfd(port_number);
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
        connfd = malloc(sizeof(int));
        *connfd = Accept(listenFd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, xacto_client_service, connfd);
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}
