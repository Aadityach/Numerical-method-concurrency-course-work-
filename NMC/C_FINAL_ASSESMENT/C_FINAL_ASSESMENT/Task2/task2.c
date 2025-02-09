#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Structure to hold thread data
typedef struct {
    int start;          // Start index for the thread
    int end;            // End index for the thread
    double result;      // Result of the thread's computation
} ThreadData;

// Function to calculate part of the Leibniz series
void *calculate_pi(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double partial_sum = 0.0;

    for (int k = data->start; k < data->end; k++) {
        partial_sum += ((k % 2 == 0) ? 1.0 : -1.0) / (2.0 * k + 1);
    }

    data->result = partial_sum;
    pthread_exit(NULL);
}

int main() {
    int num_iterations, num_threads;

    // Step 1: Get user inputs
    printf("Enter the number of iterations: ");
    scanf("%d", &num_iterations);
    printf("Enter the number of threads: ");
    scanf("%d", &num_threads);

    if (num_threads <= 0 || num_iterations <= 0) {
        printf("Error: Number of iterations and threads must be positive.\n");
        return 1;
    }

    // Step 2: Initialize threads and divide workload
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    int base_size = num_iterations / num_threads;
    int remainder = num_iterations % num_threads;
    
    int start = 0;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = start;
        thread_data[i].end = start + base_size + (i < remainder ? 1 : 0);
        thread_data[i].result = 0.0;
        start = thread_data[i].end;

        if (pthread_create(&threads[i], NULL, calculate_pi, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }

    // Step 3: Wait for all threads to complete
    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += thread_data[i].result;
    }

    // Step 4: Multiply result by 4 to get final value of Pi
    pi *= 4.0;

    // Step 5: Print the computed value of Pi
    printf("Computed value of Pi: %.15f\n", pi);

    return 0;
}
