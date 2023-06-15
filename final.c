#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 8
#define NUM_PRODUCERS 8
#define NUM_CONSUMERS 8

char buffer[BUFFER_SIZE] = {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'};
int producer_index = 0;
int consumer_index = 0;

pthread_mutex_t mutex;
pthread_cond_t buffer_not_full;
pthread_cond_t buffer_not_empty;

void* producer_function(void* arg) {
    int producer_id = *(int*)arg;
    free(arg);

    srand(time(NULL) + producer_id);

    while (1) {
        printf("Producer %d: Starts work\n", producer_id);
        usleep(rand() % 1000000); // simulate producing

        pthread_mutex_lock(&mutex);

        while (producer_index - consumer_index == BUFFER_SIZE) { // check if the buffer is full (producer a whole buffer ahead of consumer)
            pthread_cond_wait(&buffer_not_full, &mutex);
        }

        buffer[producer_index % BUFFER_SIZE] = 'O';
        printf("Producer %d: Produced at index %d | Buffer: %s\n", producer_id, producer_index % BUFFER_SIZE, buffer);
        producer_index++;

        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&mutex);
        printf("Producer %d: Ends work\n", producer_id);
    }

    return NULL;
}

void* consumer_function(void* arg) {
    int consumer_id = *(int*)arg;
    free(arg);

    srand(time(NULL) + NUM_PRODUCERS + consumer_id);

    while (1) {
        printf("Consumer %d: Starts work\n", consumer_id);
        usleep(rand() % 1000000); // simulate consuming

        pthread_mutex_lock(&mutex);

        while (producer_index == consumer_index) { // check if the buffer is empty
            pthread_cond_wait(&buffer_not_empty, &mutex);
        }
        
        buffer[consumer_index % BUFFER_SIZE] = 'X';
        printf("Consumer %d: Consumed at index %d | Buffer: %s\n", consumer_id, consumer_index % BUFFER_SIZE, buffer);
        consumer_index++;

        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&mutex);
        printf("Consumer %d: Ends work\n", consumer_id);
    }

    return NULL;
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
        pthread_create(&producer_threads[i], NULL, producer_function, (void*)producer_id);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        int* consumer_id = malloc(sizeof(int));
        *consumer_id = i + 1;
        pthread_create(&consumer_threads[i], NULL, consumer_function, (void*)consumer_id);
    }

    for(int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producer_threads[i], NULL);
    }

    for(int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumer_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);
    
    return 0;
}