#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <fstream>
#include <iomanip>
#include <algorithm>
using namespace std;

// Including generator function to grab articles
char *str_generator(void);

// Initializing Mutex for buffer access
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initializing Mutex for screen access
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;



// Defining Argument structure
struct Argument {
    int interval;
    int id;
    // int M;
};


// Defining Queue structure
struct Queue {
    int front;     // front points to front element in the queue (if any)
    int rear;      // rear points to last element in the queue
    char *arr[13]; // array to store queue elements
} buffer = {0, 0};

// Utility function to check if the buffer is full
bool isFull() {
    return (buffer.rear + 1) % 13 == buffer.front;
}

// Utility function to check if the buffer is empty
bool isEmpty() {
    return buffer.front == buffer.rear;
}

// Utility function to remove rear element from the queue
void dequeue() {

    // Check for queue underflow
    if (isEmpty()) {
        cout << "UnderFlow\nProgram Terminated\n";
        exit(EXIT_FAILURE);
    }
    buffer.front = (buffer.front + 1) % 13;
    
}

// Utility function to add an item to the queue
void enqueue(char *item) {

    // Check for queue overflow
    if (isFull()) {
        cout << "OverFlow\nProgram Terminated\n";
        exit(EXIT_FAILURE);
    }

    buffer.arr[buffer.rear] = item;
    buffer.rear = (buffer.rear + 1) % 13;

}

// Utility function to return rear element in the queue
char *peek() {

    if (isEmpty()) {
        cout << "UnderFlow\nProgram Terminated\n";
        exit(EXIT_FAILURE);
    }

    return buffer.arr[buffer.front];

}




int main(int argc, char **argv) {

    // Read input arguments and convert to integers
    int interval_A = atoi(argv[1]);
    int interval_B = atoi(argv[2]);

    // Display output first line
    cout << setw(16) << "crawler1"
         << setw(16) << "crawler2"
         << setw(16) << "crawler3"
         << setw(16) << "classifier"
         // << setw(16) << "s-manager"
         << endl;

    // Threads for crawlers and classifier
    pthread_t crawler1, crawler2, crawler3, classifier;
    // pthread_t strategy_manager;

    return 0;
}