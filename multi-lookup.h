#ifndef MULTILOOKUP_H
#define MULTILOOKUP_H

#include "util.h"
#include "array.h"



#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS  10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define MAX_ADDRESS_LENGTH 50

typedef struct data {
    
    int fileAmount;
    int threadID;
    char *serviceFile;
    char fileName[MAX_INPUT_FILES][MAX_NAME_LENGTH];
    pthread_mutex_t reqMutex;

    struct array *sharedptr;
    
    char *serverName;
    char *serverDNS;
    char *resolverFile;
    pthread_mutex_t resMutex;
    pthread_t resThreads[MAX_RESOLVER_THREADS];

    int *counter;


} data;

typedef struct signaler {
    pthread_cond_t isDone;
} signaler;



#endif 