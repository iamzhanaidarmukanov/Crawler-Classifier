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
    int M;
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



bool quit, ip_cookies_update_access;
sem_t buffer_count, buffer_size, getval_access, ip_cookies_update, cr_id_sem;
int buf_size, buf_count, ipsem_retval, crawler_id;


// 
// Crawler Handler
// 
void *crawler(void *arg) {

    struct Argument *args = (struct Argument *)arg;

    // Locking screen and getting access
    pthread_mutex_lock(&screen_mutex);
    cout << setw(16 * args->id) << "start" << endl;
    pthread_mutex_unlock(&screen_mutex);

    // Count of elements that crawler have loaded to the buffer
    int crawler_update_count = 0;


    // 
    // Start of the Busy Loop
    while (true) {

        if (crawler_update_count % (args -> M) == 0 && crawler_update_count > 0) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "rest" << endl;
            pthread_mutex_unlock(&screen_mutex);

            sem_wait(&cr_id_sem);

            crawler_id = args->id;

            sem_wait(&ip_cookies_update);

            ip_cookies_update_access = false;
            // wait for manager to update ip and cookies
            while (ip_cookies_update_access == false)
                ;

            sem_post(&cr_id_sem);

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "s-rest" << endl;
            pthread_mutex_unlock(&screen_mutex);
            
        }

        usleep(args -> interval);

        if (quit) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "quit" << endl;
            pthread_mutex_unlock(&screen_mutex);
            pthread_exit(NULL);

        }

        sem_wait(&getval_access);
        sem_getvalue(&buffer_size, &buf_size);
        int buf_size_cpy = buf_size;
        sem_post(&getval_access);

        if (buf_size == 0) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "wait" << endl;
            pthread_mutex_unlock(&screen_mutex);

            if (quit) {

                pthread_mutex_lock(&screen_mutex);
                cout << setw(16 * args->id) << "quit" << endl;
                pthread_mutex_unlock(&screen_mutex);
                pthread_exit(NULL);
                
            }

        }

        sem_wait(&buffer_size);

        if (buf_size_cpy == 0) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "s-wait" << endl;
            pthread_mutex_unlock(&screen_mutex);

        }

        if (quit) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "quit" << endl;
            pthread_mutex_unlock(&screen_mutex);
            sem_post(&buffer_size);
            pthread_exit(NULL);

        }

        char *article = str_generator();

        pthread_mutex_lock(&screen_mutex);
        cout << setw(16 * args->id) << "grab" << endl;
        pthread_mutex_unlock(&screen_mutex);

        if (quit) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * args->id) << "quit" << endl;
            pthread_mutex_unlock(&screen_mutex);
            break;

        }

        pthread_mutex_lock(&buffer_mutex);
        enqueue(article);
        pthread_mutex_unlock(&buffer_mutex);

        sem_post(&buffer_count);

        pthread_mutex_lock(&screen_mutex);
        cout << setw(16 * args->id) << "f-grab" << endl;
        pthread_mutex_unlock(&screen_mutex);

        crawler_update_count++;

    }

    return NULL;

}




vector<int> classes_count(13, 0);
int key = 0;

// 
// Classifier Handler
// 
void classifier_handler(ofstream &txt_corpus, const int &interval_B) {
    
    sem_wait(&buffer_count);

    pthread_mutex_lock(&screen_mutex);
    cout << setw(16 * 4) << "clfy" << endl;
    pthread_mutex_unlock(&screen_mutex);

    char *article_original = peek();

    vector<char> article_cpy;
    for (int i = 0; i < 50; i++) {
        if (isupper(article_original[i]) || islower(article_original[i])) {
            article_cpy.push_back((char)tolower(article_original[i]));
        }
    }

    int class_label = ((int)article_cpy.at(0) - 'a') % 13 + 1;
    classes_count.at(class_label - 1)++;
    key++;
    txt_corpus << key << " " << class_label << " " << article_original << endl;

    pthread_mutex_lock(&buffer_mutex);
    dequeue();
    pthread_mutex_unlock(&buffer_mutex);

    pthread_mutex_lock(&screen_mutex);
    cout << setw(16 * 4) << "f-clfy" << endl;
    pthread_mutex_unlock(&screen_mutex);

    sem_post(&buffer_size); 
}

void *data_classification(void *arg) {

    pthread_mutex_lock(&screen_mutex);
    cout << setw(16 * 4) << "start" << endl;
    pthread_mutex_unlock(&screen_mutex);

    int interval_B = *((int *)arg);

    ofstream txt_corpus("text_corpus.txt");

    while (true) {

        usleep(interval_B);
        sort(classes_count.begin(), classes_count.end());

        if (classes_count.at(0) >= 5) {

            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * 4 - 7) << key << "-enough" << endl;
            pthread_mutex_unlock(&screen_mutex);
            quit = true; //SEND SIGNAL for all threads to quit

            while (!isEmpty()) {
                classifier_handler(txt_corpus, interval_B);
            }

            cout << setw(16 * 4 - 7) << key << "-stored" << endl;
            cout << setw(16 * 4) << "quit" << endl;
            pthread_exit(NULL);

        }

        classifier_handler(txt_corpus, interval_B);

    }

    return NULL;

}

// 
// Update IP and Cookies Handler
// 
void *update_ip_cookies(void *arg)
{
    int interval_C = *((int *)arg);

    pthread_mutex_lock(&screen_mutex);
    cout << setw(16 * 5) << "start" << endl;
    pthread_mutex_unlock(&screen_mutex);

    while (true)
    {
        sem_wait(&getval_access);
        sem_getvalue(&ip_cookies_update, &ipsem_retval);
        sem_post(&getval_access);

        if (ipsem_retval < 1)
        {
            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * 5 - 1) << "get_cr" << crawler_id << endl;
            pthread_mutex_unlock(&screen_mutex);

            usleep(interval_C);
            //CHANGE IP AND COOKIES OF CRAWLER WITH CRAWLER_ID
            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * 5 - 1) << "up_cr" << crawler_id << endl;
            pthread_mutex_unlock(&screen_mutex);

            sem_post(&ip_cookies_update);
            ip_cookies_update_access = true;
        }

        if (quit)
        {
            pthread_mutex_lock(&screen_mutex);
            cout << setw(16 * 5) << "quit" << endl;
            pthread_mutex_unlock(&screen_mutex);
            pthread_exit(NULL);
        }
    }
    return NULL;
}


// 
// Start of main function
// 
int main(int argc, char **argv) {

    // Read input arguments and convert to integers
    int interval_A = atoi(argv[1]);
    int interval_B = atoi(argv[2]);
    int interval_C = atoi(argv[3]);
    int M = atoi(argv[4]);

    // Display output first line
    cout << setw(16) << "crawler1"
         << setw(16) << "crawler2"
         << setw(16) << "crawler3"
         << setw(16) << "classifier"
         << setw(16) << "s-manager"
         << endl;

    // Threads for crawlers and classifier
    pthread_t crawler1, crawler2, crawler3, classifier;
    pthread_t strategy_manager;


    // 
    // Initialize semaphores
    // 
    sem_init(&buffer_count, 0, 0);
    sem_init(&buffer_size, 0, 12);
    sem_init(&getval_access, 0, 1);
    sem_init(&ip_cookies_update, 0, 1);
    sem_init(&cr_id_sem, 0, 1);

    Argument args1 = {interval_A, 1, M};
    Argument args2 = {interval_A, 2, M};
    Argument args3 = {interval_A, 3, M};

    // 
    // Creating crawlers threads
    // 
    int rc = pthread_create(&crawler1, NULL, crawler, (void *)&args1);
    if (rc) {
        cout << "Error when creating thread crawler1!" << endl;
        exit(-1);
    }
    rc = pthread_create(&crawler2, NULL, crawler, (void *)&args2);
    if (rc) {
        cout << "Error when creating thread crawler2!" << endl;
        exit(-1);
    }
    rc = pthread_create(&crawler3, NULL, crawler, (void *)&args3);
    if (rc) {
        cout << "Error when creating thread crawler3!" << endl;
        exit(-1);
    }


    // 
    // Creating classifier thread
    // 
    rc = pthread_create(&classifier, NULL, data_classification, &interval_B);
    if (rc) {
        cout << "Error when creating thread classifier!" << endl;
        exit(-1);
    }


    // 
    // Running *update_ip_cookies function on the strategy_manager thread
    //
    rc = pthread_create(&strategy_manager, NULL, update_ip_cookies, &interval_C);
    if (rc) {
        cout << "Error when creating thread classifier!" << endl;
        exit(-1);
    }



    // 
    // Execution of crawler threads
    // 
    rc = pthread_join(crawler1, NULL);
    if (rc) {
        cout << "Error when joining thread crawler1!" << endl;
        exit(-1);
    }
    rc = pthread_join(crawler2, NULL);
    if (rc) {
        cout << "Error when joining thread crawler2!" << endl;
        exit(-1);
    }
    rc = pthread_join(crawler3, NULL);
    if (rc) {
        cout << "Error when joining thread crawler3!" << endl;
        exit(-1);
    }


    // 
    // Execution of classifier thread
    // 
    rc = pthread_join(classifier, NULL);
    if (rc) {
        cout << "Error when joining thread classifier!" << endl;
        exit(-1);
    }


    // 
    // Execution if strategy_manager thread
    // 
    rc = pthread_join(strategy_manager, NULL);
    if (rc) {
        cout << "Error when joining thread strategy manager!" << endl;
        exit(-1);
    }

    // 
    // Close Semaphores
    // 
    sem_destroy(&buffer_count);
    sem_destroy(&buffer_size);
    sem_destroy(&getval_access);
    sem_destroy(&ip_cookies_update);
    sem_destroy(&cr_id_sem);

    // Close Thread
    pthread_exit(NULL);

    return 0;
}