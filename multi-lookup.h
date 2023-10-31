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
    char *serviceFile;
    char fileName[MAX_INPUT_FILES][MAX_NAME_LENGTH];
    pthread_mutex_t reqMutex;
    struct array *sharedptr;

} data;

typedef struct newData {
    pthread_mutex_t resMutex;
    char *resolverFile;
} newData;

typedef struct indivThreadData {
    int threadId;
    struct data *q;
    struct newData *p;
    struct counter *c;
    struct array *sharedptr;
} indivThreadData;

typedef struct counter {
    int counter;
}counter;



#endif 