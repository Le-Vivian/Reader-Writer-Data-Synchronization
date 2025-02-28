#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//I am getting error on the min func in get_external_data so I added this line to fix it
#define min(a, b) ((a) < (b) ? (a) : (b))

void process_data(char *buffer, int bufferSizeInBytes);
int get_external_data(char *buffer, int bufferSizeInBytes);


/*********************************************************
**********************************************************
***********   DO NOT MODIFY THIS SAMPLE CODE   ***********
**********************************************************
**********************************************************/
int get_external_data(char *buffer, int bufferSizeInBytes)
{
    int status;
    int val;
	char srcString[] = "0123456789abcdefghijklmnopqrstuvwxyxABCDEFGHIJKLMNOPQRSTUVWXYZ";

	val = (int)(random()%min(bufferSizeInBytes, 62));

	if (bufferSizeInBytes < val)
		return (-1);

	strncpy(buffer, srcString, val);

	return val;
}

void process_data(char *buffer, int bufferSizeInBytes)
{
    int i;

    if(buffer)
    {
		printf("thread %i - ", pthread_self());
		for(i=0; i<bufferSizeInBytes; i++)
		{
			printf("%c", buffer[i]);
		}
		printf("\n");
		memset(buffer, 0, bufferSizeInBytes);
    }
	else
		printf("error in process data - %i\n", pthread_self());

    return;
}
/*********************************************************
**********************************************************
********   PLEASE IMPLEMENT YOUR TEST CODE BELOW   *******
**********************************************************
**********************************************************/

//TODO Define global data structures to be used
#define BUFFER_SIZE 256 // Size of each data buffer
#define QUEUE_CAPACITY 100 // Max number of items in the shared queue

typedef struct {
    char data[BUFFER_SIZE];
    int size; //Size of valid data
} DataPacket;

//Shared circular queue that holds the DataPackets
typedef struct {
    DataPacket queue[QUEUE_CAPACITY]; //The array to hold data packets.
    int front;
    int rear;
    int count; //Count the current number of items in the queue.
    pthread_mutex_t mutex; //A mutex for thread synchronization when accessing the queue.
    pthread_cond_t not_empty; 
    pthread_cond_t not_full; //Condition variables for synchronizing access to the queue when it's empty or full.
} SharedQueue;

void process_data(char *buffer, int bufferSizeInBytes);
int get_external_data(char *buffer, int bufferSizeInBytes);
void *reader_thread(void *arg);
void *writer_thread(void *arg);

//Initializes the shared queue with default values.
SharedQueue sharedQueue = {
    .front = 0,
    .rear = 0,
    .count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER,
    .not_full = PTHREAD_COND_INITIALIZER
};

/**
 * This thread is responsible for pulling data off of the shared data
 * area and processing it using the process_data() API.
 */
void *reader_thread(void *arg) {
    DataPacket packet; // Declare the packet variable to hold the dequeued data

    while (1) {
        // Lock the queue
        pthread_mutex_lock(&sharedQueue.mutex);

        // Wait until the queue is not empty
        while (sharedQueue.count == 0) {
            pthread_cond_wait(&sharedQueue.not_empty, &sharedQueue.mutex);
        }

        // Remove a packet from the queue
        packet = sharedQueue.queue[sharedQueue.front];
        sharedQueue.front = (sharedQueue.front + 1) % QUEUE_CAPACITY;
        sharedQueue.count--;

        // Signal that the queue is not full
        pthread_cond_signal(&sharedQueue.not_full);

        // Unlock the queue
        pthread_mutex_unlock(&sharedQueue.mutex);

        // Process the data
        process_data(packet.data, packet.size);
    }

    return NULL;
}


/**
 * This thread is responsible for pulling data from a device using
 * the get_external_data() API and placing it into a shared area
 * for later processing by one of the reader threads.
 */
void *writer_thread(void *arg) {

	 DataPacket packet;

    while (1) {
        // Get data from the external source
        packet.size = get_external_data(packet.data, BUFFER_SIZE);
        if (packet.size < 0) {
            fprintf(stderr, "Error: Failed to get data from external source.\n");
            continue;
        }

        // Lock the queue
        pthread_mutex_lock(&sharedQueue.mutex);

        // Wait until the queue is not full
        while (sharedQueue.count == QUEUE_CAPACITY) {
            pthread_cond_wait(&sharedQueue.not_full, &sharedQueue.mutex);
        }

        // Add the packet to the queue
        sharedQueue.queue[sharedQueue.rear] = packet;
        sharedQueue.rear = (sharedQueue.rear + 1) % QUEUE_CAPACITY;
        sharedQueue.count++;

        // Signal that the queue is not empty
        pthread_cond_signal(&sharedQueue.not_empty);

        // Unlock the queue
        pthread_mutex_unlock(&sharedQueue.mutex);
    }

    return NULL;
}

//N writers and M readers
#define M 10
#define N 20

int main(int argc, char **argv) {
	 pthread_t readers[M], writers[N];
    int i;

    // Create reader threads
    for (i = 0; i < M; i++) {
        pthread_create(&readers[i], NULL, reader_thread, NULL);
    }

    // Create writer threads
    for (i = 0; i < N; i++) {
        pthread_create(&writers[i], NULL, writer_thread, NULL);
    }

    // Wait for threads to complete 
    for (i = 0; i < M; i++) {
        pthread_join(readers[i], NULL);
    }

    for (i = 0; i < N; i++) {
        pthread_join(writers[i], NULL);
    }

    return 0;
}