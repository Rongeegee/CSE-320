#include "client_registry.h"
#include <stdlib.h>
#include <semaphore.h>
#include <sys/socket.h>
#include "csapp.h"

typedef struct client_registry{
    int buf[1024];
    int counter;
    sem_t mutex;
    sem_t pin;
    sem_t client;
}CLIENT_REGISTRY;


/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry.
 */
CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY* client_registry = malloc(sizeof(CLIENT_REGISTRY));
    for(int i=0; i< 1024; i++){
        client_registry->buf[i] = 0;
    }
    client_registry->counter = 0;
    sem_init(&client_registry->mutex,0,1);
    sem_init(&client_registry->pin,0,1024);
    sem_init(&client_registry->client,0,0);
    return client_registry;
}

/*
 * Finalize a client registry.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr){
    free(cr);
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 */
void creg_register(CLIENT_REGISTRY *cr, int fd){
    P(&cr->pin);
    P(&cr->mutex);
    if(cr->counter == 1024){
        return;
    }
    for(int i=0; i< 1024; i++){
        if(cr->buf[i] == 0){
            cr->buf[i] = fd;
            cr->counter++;
            break;
        }
    }

    V(&cr->mutex);
    V(&cr->client);
}

/*
 * Unregister a client file descriptor, alerting anybody waiting
 * for the registered set to become empty.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be unregistered.
 */
void creg_unregister(CLIENT_REGISTRY *cr, int fd){
    P(&cr->client);
    P(&cr->mutex);
    for(int i = 0;i < 1024;i++){
        if(cr->buf[i] == fd){
            cr->buf[i] = 0;
            break;
        }
    }
    cr->counter--;
    V(&cr->mutex);
    V(&cr->pin);
}

/*
 * A thread calling this function will block in the call until
 * the number of registered clients has reached zero, at which
 * point the function will return.
 *
 * @param cr  The client registry.
 */
void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    P(&cr->mutex);
    while(cr->counter > 0);
    V(&cr->mutex);
}


/*
 * Shut down all the currently registered client file descriptors.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr){
    P(&cr->mutex);
    for(int i = 0; i < 1024;i++){
        if(cr->buf[i] != 0){
             shutdown(cr->buf[i],SHUT_RDWR);
        }

    }
    V(&cr->mutex);
}
