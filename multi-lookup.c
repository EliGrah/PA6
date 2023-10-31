#include "multi-lookup.h"
#include <sys/time.h>



int array_initAgain(data *q) {
    pthread_mutex_init(&q->reqMutex, NULL);
    q->fileAmount = 0;
    q->serviceFile = NULL;
    q->sharedptr = NULL;
    q->countptr = NULL;


    q->resolverFile = NULL;
    pthread_mutex_init(&q->resMutex, NULL);
    return 0;
}



void *reqThread(void *arg) {
    indivThreadData *thread = (indivThreadData*)arg;
    int threadID = thread->threadId;
    char address[MAX_ADDRESS_LENGTH];

    for(int i = 0; (threadID + i) < thread->q->fileAmount; i = i + 10){

        FILE *fp = fopen(thread->q->fileName[threadID + i], "r");

        if(fp == NULL) {
            fprintf(stderr, "invalid file %s\n", thread->q->fileName[threadID + i]); 
        } else {
        
            while(fgets(address, MAX_ADDRESS_LENGTH, fp) != NULL) {
                array_put(thread->q->sharedptr, address);

                pthread_mutex_lock(&thread->q->reqMutex);

                FILE *sp = fopen(thread->q->serviceFile, "a");
                if (sp == NULL) {
                }
                fprintf(sp, "%s", address);
                fclose(sp);

                pthread_mutex_unlock(&thread->q->reqMutex);
            }
        }

        fclose(fp);
    }

    pthread_mutex_lock(&thread->q->reqMutex);
    thread->q->countptr--;
    pthread_mutex_unlock(&thread->q->reqMutex);
    fprintf(stdout, "Requester thread %d is done\n", threadID);

    pthread_exit(NULL);
}

void *resThread(void *arg){

    data *q = (data*)arg;
    char *serverName = (char *)malloc(MAX_NAME_LENGTH);
    char ip[MAX_IP_LENGTH];
    
    while(q->countptr != 0 && (q->sharedptr->front - q->sharedptr->back) != 0){
        
        array_get(q->sharedptr, serverName);

        dnslookup(serverName, ip, MAX_IP_LENGTH);

        pthread_mutex_lock(&q->resMutex);

        FILE *sp = fopen(q->resolverFile, "a");
        if (sp == NULL) {
            fprintf(stderr, "failed to open service file\n");              
        }
        fprintf(sp, "%s\n", ip);
        fclose(sp);

        pthread_mutex_unlock(&q->resMutex);
    }
    free(serverName);
    pthread_exit(NULL);
}


void array_freeAgain(data *q) {
    pthread_mutex_destroy(&q->resMutex);
    pthread_mutex_destroy(&q->reqMutex);
}




int main(int argc, char *argv[]) {
    //referenced https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/times.html#:~:text=The%20gettimeofday%20system%20call&text=h%3E%20int%20gettimeofday(struct%20timeval,UTC%20(Greenwich%20Mean%20Time).
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, 0);


    if(argc < 6) {
        fprintf(stdout, "Usage: Not enough arguments");
        return 1;
    }
    
    int reqNum = MAX_REQUESTER_THREADS;
    int resNum = MAX_RESOLVER_THREADS;
    int fileNum = argc - 7;
    char *reqFile = argv[3];
    char *resFile = argv[4];
    int counter[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    if(fileNum > MAX_ADDRESS_LENGTH) {
        fprintf(stderr, "error: too many files");
    }
    
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
    q->countptr = &counter[0];
    array_init(q->sharedptr);

    
    for (int i = 0; i < fileNum - 1; i++) {
        strcpy(q->fileName[i], argv[i + 5]);
    }


    pthread_t reqThreads[reqNum];
    indivThreadData reqThreadData[reqNum];
    
    for(int i = 0; i < reqNum; i++) {
        reqThreadData[i].threadId = i;
        reqThreadData[i].q = q;
        pthread_create(&reqThreads[i], NULL, reqThread, &reqThreadData[i]);
    }


    pthread_t resThreads[resNum];
    for(int i = 0; i < resNum; i++) {
        pthread_create(&resThreads[i], NULL, resThread, q);
    }


    for(int i = 0; i < reqNum; i++) {
        pthread_join(reqThreads[i], NULL);
    }
    for(int i = 0; i < reqNum; i++) {
        pthread_join(resThreads[i], NULL);
    } 

    array_free(q->sharedptr);
    array_freeAgain(q);

    gettimeofday(&end, 0);
    long seconds, microsec;
    double time;
    seconds = end.tv_sec - start.tv_sec;
    microsec = end.tv_usec - start.tv_usec;
    time = seconds + microsec / 1000000.0;
    fprintf(stdout, "%s: total time: %f seconds\n", argv[0], time);
    return 0;

}

