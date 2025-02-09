#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define NUM_FILES 3       // Number of input files
#define OUTPUT_FILE "primes_output.txt"

// Structure to hold thread data
typedef struct {
    int *numbers;         // Array of numbers to process
    int start;            // Start index of slice
    int end;              // End index of slice
    int *primes;          // Array to store prime numbers
    int *prime_count;     // Pointer to store the count of primes
} ThreadData;

// Function to check if a number is prime
int is_prime(int num) {
    if (num <= 1) return 0;
    for (int i = 2; i <= sqrt(num); i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

// Thread function to find primes in a slice of data
void *find_primes(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int count = 0;

    for (int i = data->start; i < data->end; i++) {
        if (is_prime(data->numbers[i])) {
            data->primes[count++] = data->numbers[i];
        }
    }

    *data->prime_count = count;
    pthread_exit(NULL);
}

int main() {
    // Input file names
    const char *filenames[NUM_FILES] = {"PrimeData1.txt", "PrimeData2.txt", "PrimeData3.txt"};
    int total_numbers = 0;

    // Step 1: Read numbers from files
    int *numbers = NULL;  // Dynamically allocated array for all numbers
    for (int i = 0; i < NUM_FILES; i++) {
        FILE *file = fopen(filenames[i], "r");
        if (!file) {
            printf("Error: Could not open file %s\n", filenames[i]);
            return 1;
        }

        int num;
        while (fscanf(file, "%d", &num) == 1) {
            numbers = realloc(numbers, (total_numbers + 1) * sizeof(int));
            numbers[total_numbers++] = num;
        }
        fclose(file);
    }

    // Step 2: Set up multithreading
    int num_threads = 4;  // Number of threads
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    int slice_size = total_numbers / num_threads;
    int remainder = total_numbers % num_threads;

    // Allocate memory for prime results and counts
    int **primes = malloc(num_threads * sizeof(int *));
    int *prime_counts = malloc(num_threads * sizeof(int));

    for (int i = 0; i < num_threads; i++) {
        int start = i * slice_size;
        int end = (i == num_threads - 1) ? total_numbers : start + slice_size;

        thread_data[i].numbers = numbers;
        thread_data[i].start = start;
        thread_data[i].end = end;

        thread_data[i].primes = malloc((end - start) * sizeof(int));
        thread_data[i].prime_count = &prime_counts[i];

        pthread_create(&threads[i], NULL, find_primes, &thread_data[i]);
    }

    // Step 3: Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Step 4: Combine results and write to output file
FILE *output_file = fopen(OUTPUT_FILE, "w");
if (!output_file) {
    printf("Error: Could not open output file.\n");
    return 1;
}

int total_primes = 0;
for (int i = 0; i < num_threads; i++) {
    total_primes += prime_counts[i];
}

// Write the total number of prime numbers first
fprintf(output_file, "Total number of prime numbers: %d\n\n", total_primes);
fprintf(output_file, "Prime numbers found:\n");

// Write all the prime numbers
for (int i = 0; i < num_threads; i++) {
    for (int j = 0; j < prime_counts[i]; j++) {
        fprintf(output_file, "%d\n", thread_data[i].primes[j]);
    }
}

fclose(output_file);

    // Step 5: Free allocated memory
    for (int i = 0; i < num_threads; i++) {
        free(thread_data[i].primes);
    }
    free(primes);
    free(prime_counts);
    free(numbers);

    printf("Prime numbers written to %s\n", OUTPUT_FILE);
    return 0;
}
