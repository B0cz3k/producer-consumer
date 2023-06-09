#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 5
#define WAIT_TIME (rand() % 1000000)

char buffer[BUFFER_SIZE] = {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'};
int buffer_index = 0;

pthread_mutex_t mutex;
pthread_cond_t buffer_not_full;
pthread_cond_t buffer_not_empty;


void* producer(void* arg) {
    int producer_id = *(int*)arg;
    free(arg);

    while (1) {
        usleep(WAIT_TIME);  // simulates producing
        
        pthread_mutex_lock(&mutex);

        while (buffer_index == BUFFER_SIZE) { // check if the buffer is full
            pthread_cond_wait(&buffer_not_full, &mutex);
        }

        buffer[buffer_index++] = 'O';
        printf("Producer %d:\t%s\n", producer_id, buffer);

        pthread_cond_signal(&buffer_not_empty);

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

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
    pthread_t producer_threads[NUM_PRODUCERS];
    pthread_t consumer_threads[NUM_CONSUMERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        int* producer_id = malloc(sizeof(int));
        *producer_id = i + 1;
        pthread_create(&producer_threads[i], NULL, producer, (void*)producer_id);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        int* consumer_id = malloc(sizeof(int));
        *consumer_id = i + 1;
        pthread_create(&consumer_threads[i], NULL, consumer, (void*)consumer_id);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producer_threads[i], NULL);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumer_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);

    return 0;
}