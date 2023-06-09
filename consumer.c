#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 10
#define NUM_CONSUMERS 5
#define WAIT_TIME (rand() % 1000000)

char buffer[BUFFER_SIZE];
int buffer_index;

pthread_mutex_t mutex;
pthread_cond_t buffer_not_full;
pthread_cond_t buffer_not_empty;


void* consumer(void* arg) {
    int consumer_id = *(int*)arg;
    free(arg);

    while (1) {
        usleep(WAIT_TIME);  // simulates consuming

        pthread_mutex_lock(&mutex);

        while (buffer_index == 0) { // check if the buffer is empty
            pthread_cond_wait(&buffer_not_empty, &mutex);
        }

        buffer[--buffer_index] = 'X';
        printf("Consumer %d:\t%s\n", consumer_id, buffer);

        pthread_cond_signal(&buffer_not_full);

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t consumer_threads[NUM_CONSUMERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        int* consumer_id = malloc(sizeof(int));
        *consumer_id = i + 1;
        pthread_create(&consumer_threads[i], NULL, consumer, (void*)consumer_id);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumer_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);

    return 0;
}