#include <stdio.h>
#include <pthread.h>

int counter = 0;          // Shared counter
int check = 0;            // Variable to store the first thread to reach 50
pthread_mutex_t lock;     


void increment_counter(int thread_id) {
    while (1) {
        pthread_mutex_lock(&lock);  
        if (counter >= 50) {       
            pthread_mutex_unlock(&lock);
            break;
        }
        counter++;
        if (counter == 50 && check == 0) { 
            check = thread_id;
        }
        pthread_mutex_unlock(&lock); 
    }
}

// Thread functions
void* count_thread1(void* arg) {
    increment_counter(1); 
    return NULL;
}

void* count_thread2(void* arg) {
    increment_counter(2); 
    return NULL;
}

void* count_thread3(void* arg) {
    increment_counter(3); 
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;


    pthread_mutex_init(&lock, NULL);

  
    pthread_create(&thread1, NULL, count_thread1, NULL);
    pthread_create(&thread2, NULL, count_thread2, NULL);
    pthread_create(&thread3, NULL, count_thread3, NULL);


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);


    printf("Thread %d was the first to reach 50.\n", check);


    pthread_mutex_destroy(&lock);

    return 0;
}
