#include "protocol.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
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
    uint8_t type = htonl(pkt->type);
    if(write(fd,&type,sizeof(uint8_t)) == -1)
        return -1;
    uint8_t status = htonl(pkt->status);
    if(write(fd,&status,sizeof(uint8_t)) == -1)
        return -1;
    uint8_t null = htonl(pkt->null);
    if(write(fd,&null,sizeof(uint8_t)) == -1)
        return -1;
    uint32_t size = htonl(pkt->size);
    if(write(fd,&size,sizeof(uint32_t)) == -1)
        return -1;
    uint32_t timestamp_sec = htonl(pkt->timestamp_sec);
    if(write(fd,&timestamp_sec,sizeof(uint32_t)) == -1)
        return -1;
    uint32_t timestamp_nsec = htonl(pkt->timestamp_nsec);
    if(write(fd,&timestamp_nsec,sizeof(uint32_t)) == -1)
        return -1;
    if(size > 0){
        if(write(fd,data,size) == -1)
            return -1;
    }
    return 0;
}