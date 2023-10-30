#ifndef ARRAY_H
#define ARRAY_H

#define ARRAY_SIZE 8
#define MAX_NAME_LENGTH 20


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct array {
    char serverNames[ARRAY_SIZE][MAX_NAME_LENGTH];
    int front;
    int back;
    pthread_mutex_t mutex;
    pthread_cond_t notFull;
    pthread_cond_t notEmpty;

} array;

int array_put(array*, char*);
int array_init(array*);
int array_get(array*, char**);
void array_free(array*);
#endif