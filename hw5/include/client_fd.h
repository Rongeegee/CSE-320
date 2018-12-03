#ifndef CLIENT_FD_H
#define CLIENT_FD_H


typedef struct connFd{
    int fd;
    struct connFd* nextFd;
    struct connFd* prevFd;
} connFd;




#endif