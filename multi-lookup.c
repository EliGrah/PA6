#include "multi-lookup.h"
#include <sys/time.h>



int array_initAgain(data *q, counter *c, newData *p) {
    pthread_mutex_init(&q->reqMutex, NULL);
    q->fileAmount = 0;
    q->serviceFile = NULL;
    q->sharedptr = NULL;
    
    c->counter = 0;
    p->resolverFile = NULL;
    pthread_mutex_init(&p->resMutex, NULL);
    return 0;
}



void *reqThread(void *arg) {
    indivThreadData *thread = (indivThreadData*)arg;
    int fileAmount = thread->q->fileAmount;
    int threadID = thread->threadId;
    char address[MAX_ADDRESS_LENGTH];
    int numLooked = 0;
    int reqNum = thread->c->counter;
    char *serviceFile = thread->q->serviceFile;

    pthread_t actualID = pthread_self();
    for(int i = 0; (threadID + i) < fileAmount; i = i + reqNum){
        FILE *fp = fopen(thread->q->fileName[threadID + i], "r");
        numLooked++;
        if(fp == NULL) {
            fprintf(stderr, "invalid file %s\n", thread->q->fileName[threadID + i]); 
        } else {
            while(fgets(address, MAX_ADDRESS_LENGTH, fp) != NULL) {
            
                array_put(thread->sharedptr, address);
                FILE *sp = fopen(serviceFile, "a");
                pthread_mutex_lock(&thread->q->reqMutex);
                if (sp == NULL) { 
                    fprintf(stderr, "failed to open service file\n");  
                }
                fprintf(sp, "%s", address);
                pthread_mutex_unlock(&thread->q->reqMutex);
                fclose(sp);
            }
        }
        fclose(fp);
    }

    thread->c->counter--;
    if(numLooked > 0) {
        fprintf(stdout, "thread %p serviced %d files\n", actualID, numLooked);
    }
    printf("counter = %d\n", thread->c->counter);
    pthread_exit(NULL);
}

void *resThread(void *arg){
    
    indivThreadData *thread = (indivThreadData*)arg;
    char *serverName = (char *)malloc(MAX_NAME_LENGTH);
    char ip[MAX_IP_LENGTH];
    int serviced = 0;
    int check = 0;
    pthread_t actualID = pthread_self();
    char *resolverFile = thread->p->resolverFile;


    
    while(thread->c->counter != 0 || (thread->sharedptr->front - thread->sharedptr->back) != 0){
        
        array_get(thread->sharedptr, serverName);

        check = dnslookup(serverName, ip, MAX_IP_LENGTH);
        if(check != 0){
            strcpy(ip, "NOT_RESOLVED");
            serviced--;
        }
        serviced++;
        FILE *sp = fopen(resolverFile, "a");
        pthread_mutex_lock(&thread->p->resMutex);
        if (sp == NULL) {
            fprintf(stderr, "failed to open service file\n");              
        }
        fprintf(sp, "%s, %s\n", serverName, ip);
        pthread_mutex_unlock(&thread->p->resMutex);
        fclose(sp);
    }
    free(serverName);
    if(serviced > 0) {
        fprintf(stdout, "thread %p resolved %d hosts\n", actualID, serviced);
    }
    pthread_exit(NULL);
}


void array_freeAgain(data *q, newData *p) {
    pthread_mutex_destroy(&p->resMutex);
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
    
    int reqNum = atoi(argv[1]);
    int resNum = atoi(argv[2]);
    int fileNum = argc - 5;
    char *reqFile = argv[3];
    char *resFile = argv[4];
    

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
    newData restData;
    counter count;
    array sharedArray;

    
    data *q = &allData;
    newData *p = &restData;
    counter *c = &count;
    
    
    array_initAgain(q, c, p);
    q->sharedptr = &sharedArray;
    q->fileAmount = fileNum;
    q->serviceFile = reqFile;
    p->resolverFile = resFile;
    c->counter = reqNum;

    array_init(q->sharedptr); 

    
    for (int i = 0; i < fileNum; i++) {
        strcpy(q->fileName[i], argv[i + 5]);
    }   


    pthread_t reqThreads[reqNum];
    indivThreadData reqThreadData[reqNum];
    for(int i = 0; i < reqNum; i++) {
        reqThreadData[i].threadId = i;
        reqThreadData[i].q = q;
        reqThreadData[i].c = c;
        reqThreadData[i].sharedptr = q->sharedptr;

        
        pthread_create(&reqThreads[i], NULL, reqThread, &reqThreadData[i]);
    }

    pthread_t resThreads[resNum];
    indivThreadData resThreadData[resNum];

    for(int i = 0; i < resNum; i++) {
        resThreadData[i].p = p;
        resThreadData[i].c = c;
        resThreadData[i].sharedptr = q->sharedptr;

        pthread_create(&resThreads[i], NULL, resThread, &resThreadData[i]);
    }

    for(int i = 0; i < reqNum; i++) {
        pthread_join(reqThreads[i], NULL);
    }
    for(int i = 0; i < reqNum; i++) {
        pthread_join(resThreads[i], NULL);
    } 
    array_free(q->sharedptr);
    array_freeAgain(q, p);

    gettimeofday(&end, 0);
    long seconds, microsec;
    double time;
    seconds = end.tv_sec - start.tv_sec;
    microsec = end.tv_usec - start.tv_usec;
    time = seconds + microsec / 1000000.0;
    fprintf(stdout, "%s: total time: %f seconds\n", argv[0], time);
    return 0;

}

