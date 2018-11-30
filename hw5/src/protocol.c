#include "protocol.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "csapp.h"

/*
 * Send a packet, followed by an associated data payload, if any.
 * Multi-byte fields in the packet are converted to network byte order
 * before sending.  The structure passed to this function may be modified
 * as a result of this conversion process.
 *
 * @param fd  The file descriptor on which packet is to be sent.
 * @param pkt  The fixed-size part of the packet, with multi-byte fields
 *   in host byte order
 * @param data  The payload for data packet, or NULL.  A NULL value used
 *   here for a data packet specifies the transmission of a special null
 *   data value, which has no content.
 * @return  0 in case of successful transmission, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 */
int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data){
    int size = pkt->size;
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);
    if(rio_writen(fd,pkt,sizeof(XACTO_PACKET)) < 0)
        return -1;
    if(size > 0){
        if(rio_writen(fd,data,pkt->size) < 0)
            return -1;
    }
    return 0;
}

/*
 * Receive a packet, blocking until one is available.
 * The returned structure has its multi-byte fields in host byte order.
 *
 * @param fd  The file descriptor from which the packet is to be received.
 * @param pkt  Pointer to caller-supplied storage for the fixed-size
 *   portion of the packet.
 * @param datap  Pointer to variable into which to store a pointer to any
 *   payload received.
 * @return  0 in case of successful reception, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 *
 * If the returned payload pointer is non-NULL, then the caller assumes
 * responsibility for freeing the storage.
 */

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
    if(rio_readn(fd,pkt,sizeof(XACTO_PACKET)) <= 0)
        return -1;
    pkt->size = ntohl(pkt->size);
    pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
    pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);
    if(pkt->size > 0){
        *datap = malloc(pkt->size);
        if(rio_readn(fd,*datap,pkt->size) <= 0)
            return -1;
    }
    return 0;
}
