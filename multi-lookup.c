#include "multi-lookup.h"




int array_initAgain(data *q) {
    pthread_mutex_init(&q->reqMutex, NULL);
    q->fileAmount = 0;
    q->threadID = 0;
    q->serviceFile = NULL;
    q->sharedptr = NULL;

    q->counter = NULL;

    q->serverName = NULL;
    q->serverDNS = NULL;
    q->resolverFile = NULL;
    pthread_mutex_init(&q->resMutex, NULL);
    return 0;
}

void *reqThread(void *arg) {
   
    data *q = (data*)arg;
    char address[MAX_ADDRESS_LENGTH];

    for(int i = 0; (q->threadID + i) < q->fileAmount; i = i + 10){

        FILE *fp = fopen(q->fileName[q->threadID + i], "r");

        if(fp == NULL) {
            fprintf(stderr, "failed to open file\n"); 
        }
        
        while(fgets(address, MAX_ADDRESS_LENGTH, fp) != NULL) {
            array_put(q->sharedptr, address);

            pthread_mutex_lock(&q->reqMutex);

            FILE *sp = fopen(q->serviceFile, "a");
            if (sp == NULL) {
                fprintf(stderr, "failed to open service file\n");             
            }
            fprintf(sp, "%s", address);
            fclose(sp);

            pthread_mutex_unlock(&q->reqMutex);
        }

        fclose(fp);
    }

    pthread_mutex_lock(&q->reqMutex);
    q->counter--;
    pthread_mutex_unlock(&q->reqMutex);

    pthread_exit(NULL);
}

void *resThread(void *arg){
    char *address[MAX_ADDRESS_LENGTH];
    char *ip[MAX_IP_LENGTH];

    int array_get(array* sharedArray, char** address);


}


void array_freeAgain(data *q) {
    pthread_mutex_destroy(&q->resMutex);
    pthread_mutex_destroy(&q->reqMutex);
}



int main(int argc, char *argv[]) {

    //imma use the max screw the user
    
    int reqNum = MAX_REQUESTER_THREADS;
    int resNum = MAX_RESOLVER_THREADS;
    int fileNum = argc - 5;
    char *reqFile = argv[3];
    char *resFile = argv[4];
    
    FILE *test;

    test = fopen(reqFile, "w");
    if(test == NULL){
        fprintf(stderr, "Requester file failed to open\n");
    }
    fclose(test);

    test = fopen(resFile, "w");
    if(test == NULL){
        fprintf(stderr, "Resolver file failed to open\n");
    }
    fclose(test);


    data allData;
    array sharedArray;

    data *q = &allData;
    
    
    
    array_initAgain(q);
    q->sharedptr = &sharedArray;
    q->fileAmount = fileNum;
    q->serviceFile = reqFile;
    q->resolverFile = resFile;
    array_init(q->sharedptr);

    q->counter = (int *)malloc(sizeof(int));
    if(q->counter == NULL){
        fprintf(stderr, "malloc failed");
    }
    q->counter = 10;
    

    for (int i = 0; i < fileNum - 1; i++) {
        strcpy(q->fileName[i], argv[i + 5]);
    }
    
    pthread_t reqThreads[reqNum];

    for(int i = 0; i < reqNum; i++) {
        q->threadID = i;
        pthread_create(&reqThreads[i], NULL, reqThread, q);
    }

    pthread_t resThreads[resNum];

    for(int i = 0; i < resNum; i++) {
        q->threadID = i;
        pthread_create(&resThreads[i], NULL, resThread, q);
    }


    for(int i = 0; i < reqNum; i++) {
        pthread_join(reqThreads[i], NULL);
    }

    for(int i = 0; i < reqNum; i++) {
        pthread_join(resThreads[i], NULL);
    } 

    free(q->counter);
    array_free(q->sharedptr);
    array_freeAgain(q);
    

}

