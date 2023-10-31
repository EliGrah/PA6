#include "array.h"

void remove_newLine(char *string) {
    int strLength = strlen(string);

    if(string[strLength - 1] == '\n') {
        string[strLength - 1] = '\0';
    }
}

int array_init(array *s) {

    s->front = 0;
    s->back = 0;
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->notFull, NULL);
    pthread_cond_init(&s->notEmpty, NULL);
    

    return 0;
}

int array_put (array *s, char *hostname) {
    pthread_mutex_lock(&s->mutex);

    while((s->front - s->back) == ARRAY_SIZE) {
        pthread_cond_wait(&s->notFull, &s->mutex);
    }

    strcpy(s->serverNames[s->front % ARRAY_SIZE], hostname);
    s->front++;

    pthread_cond_signal(&s->notEmpty);
    pthread_mutex_unlock(&s->mutex);

    return 0;
}

int array_get (array *s, char *hostname) {
    remove_newLine(s->serverNames[s->back % ARRAY_SIZE]);
    
    pthread_mutex_lock(&s->mutex);
    while((s->front - s->back) == 0) {
        pthread_cond_wait(&s->notEmpty, &s->mutex);
    }

    strcpy(hostname, s->serverNames[s->back % ARRAY_SIZE]);

    s->back++;

    pthread_cond_signal(&s->notFull);
    pthread_mutex_unlock(&s->mutex);
    
    return 0;
}

void array_free(array *s) {
    pthread_mutex_destroy(&s->mutex);
    pthread_cond_destroy(&s->notEmpty);
    pthread_cond_destroy(&s->notFull);

}