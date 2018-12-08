#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "transaction.h"
#include "protocol.h"
#include "data.h"
#include "store.h"
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

CLIENT_REGISTRY *client_registry;

/*
 * Thread function for the thread that handles client requests.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This pointer must be freed once the file
 * descriptor has been retrieved.
 */
void *xacto_client_service(void *arg){
    int fd = *((int*)(arg));
    free(arg);
    pthread_detach(pthread_self());
    creg_register(client_registry,fd);

    while(1){
        TRANSACTION* transaction = trans_create();
        XACTO_PACKET* receivePkt = malloc(sizeof(XACTO_PACKET));
        XACTO_PACKET* sendPkt = malloc(sizeof(XACTO_PACKET));
        void **datap = malloc(sizeof(void**));
        int receiveSuccess =  proto_recv_packet(fd,receivePkt,datap);
       if(receiveSuccess < 0){
            free(receivePkt);
            free(sendPkt);
            free(datap);
            close(fd);
            break;

       }
        if(receivePkt->type == XACTO_PUT_PKT){
            proto_recv_packet(fd,receivePkt,datap);
            BLOB* blob = blob_create(*datap,strlen(*datap));
            KEY* key = key_create(blob);
            proto_recv_packet(fd,receivePkt,datap);
            BLOB* dataBlob = blob_create(*datap,strlen(*datap));
            store_put(transaction,key,dataBlob);
            sendPkt->type = XACTO_REPLY_PKT;
            sendPkt->status = transaction->status;
            sendPkt->null = 0;
            sendPkt->size = 0;
            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time);
            sendPkt->timestamp_sec = time.tv_sec;
            sendPkt->timestamp_nsec = time.tv_nsec;
            proto_send_packet(fd,sendPkt,datap);
        }
        else if(receivePkt->type == XACTO_GET_PKT){
            proto_recv_packet(fd,receivePkt,datap);
            BLOB* blob = blob_create(*datap,receivePkt->size);
            BLOB** blobP = malloc(sizeof(BLOB**));
            KEY* key = key_create(blob);
            store_get(transaction,key,blobP);
            sendPkt->type = XACTO_REPLY_PKT;
            sendPkt->status = transaction->status;
            sendPkt->null = 0;
            sendPkt->size = 0;
            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time);
            sendPkt->timestamp_sec = time.tv_sec;
            sendPkt->timestamp_nsec = time.tv_nsec;
            proto_send_packet(fd,sendPkt,NULL);



            sendPkt->type = XACTO_DATA_PKT;
            sendPkt->status = transaction->status;
            sendPkt->null = 0;
            sendPkt->size = 0;
            clock_gettime(CLOCK_REALTIME, &time);
            sendPkt->timestamp_sec = time.tv_sec;
            sendPkt->timestamp_nsec = time.tv_nsec;
            if(*blobP == NULL){
                sendPkt->null = 1;
                sendPkt->size = 0;
                proto_send_packet(fd,sendPkt, NULL);
            }else{
                sendPkt->null = 0;
                sendPkt->size = (*blobP)->size;
                proto_send_packet(fd,sendPkt, (*blobP)->content);
            }
        }
        else if(receivePkt->type == XACTO_COMMIT_PKT){
            trans_commit(transaction);
            sendPkt->type = XACTO_REPLY_PKT;
            sendPkt->status = transaction->status;
            sendPkt->null = 0;
            sendPkt->size = 0;
            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time);
            sendPkt->timestamp_sec = time.tv_sec;
            sendPkt->timestamp_nsec = time.tv_nsec;
            proto_send_packet(fd,sendPkt,datap);
        }

        free(sendPkt);
        free(receivePkt);
        free(datap);
        if(transaction->status == 1 || transaction->status == 2){
            close(fd);
            break;
        }
    }
    creg_unregister(client_registry,fd);
    close(fd);
    return NULL;
}