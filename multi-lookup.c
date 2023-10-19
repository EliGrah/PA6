#include "multi-lookup.h"


pthread_mutex_t file_mutex;

void threadFunction(int thread_id) {
    
    pthread_mutex_lock(&file_mutex);

    FILE *fp;
    fp = fopen("test.txt", "a");
    fputc((char)thread_id, fp);


    pthread_mutex_unlock(&file_mutex);
}

int main() {
    for(int i = 0; i < 5; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, threadFunction, &i);
    }
}