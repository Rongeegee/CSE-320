#include "client_registry.h"
#include "client_fd.h"
#include <stdlib.h>
#include <semaphore.h>
#include <sys/socket.h>
#include "csapp.h"

typedef struct client_registry{
    connFd* connHead;
    int counter;
    int terminated;
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
    CLIENT_REGISTRY* client_registry = malloc(sizeof(client_registry));
    client_registry->connHead = NULL;
    client_registry->counter = 0;
    client_registry->terminated = 0;
    Sem_init(&client_registry->mutex,0,1);
    Sem_init(&client_registry->pin,0,1024);
    Sem_init(&client_registry->client,0,0);
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
    if(cr->connHead == NULL){
        cr->connHead = NULL;
        cr->connHead->fd = fd;
        cr->connHead->nextFd = NULL;
        cr->connHead->prevFd = NULL;
        cr->counter++;
        V(&cr->mutex);
        return;
    }
    connFd* fileDescriptor = cr->connHead;
    while(fileDescriptor->nextFd != NULL){
        fileDescriptor = fileDescriptor->nextFd;
    }
    connFd* newFd = malloc(sizeof(connFd));
    newFd->fd = fd;
    newFd->nextFd = NULL;
    newFd->prevFd = fileDescriptor;
    fileDescriptor->nextFd = newFd;
    cr->counter++;
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
    if(cr->connHead->fd == fd){
        connFd* tempFd = cr->connHead;
        cr->connHead = cr->connHead->nextFd;
        tempFd->nextFd = NULL;
        tempFd->prevFd = NULL;
        tempFd = NULL;
        free(tempFd);
        cr->connHead->prevFd = NULL;
        cr->counter--;
        V(&cr->mutex);
        V(&cr->pin);
        return;
    }
    connFd* fileDescriptor = cr->connHead;
    while(fileDescriptor != NULL){
        if(fileDescriptor->fd == fd){
            if(fileDescriptor->nextFd == NULL){
                connFd* prevDescriptor = fileDescriptor->prevFd;
                prevDescriptor->nextFd = NULL;
                fileDescriptor->prevFd = NULL;
                free(fileDescriptor);
                cr->counter--;
                V(&cr->mutex);
                V(&cr->pin);
                return;
            }
            else{
                connFd* prevFd = fileDescriptor->prevFd;
                connFd* nextFd = fileDescriptor->nextFd;
                prevFd->nextFd = nextFd;
                nextFd->prevFd = prevFd;
                fileDescriptor->nextFd = NULL;
                fileDescriptor->prevFd = NULL;
                free(fileDescriptor);
                cr->counter--;
                V(&cr->mutex);
                V(&cr->pin);
                return;
            }
        }
        fileDescriptor = fileDescriptor->nextFd;
    }
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
    while(&cr->counter > 0);
    V(&cr->mutex);
}


/*
 * Shut down all the currently registered client file descriptors.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr){
    P(&cr->mutex);
    connFd* connection = cr->connHead;
    while(connection != NULL){
        shutdown(connection->fd,SHUT_RDWR);
        connection = connection->nextFd;
    }
    V(&cr->mutex);
}
