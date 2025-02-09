#include <stdio.h>
#include <stdlib.h>

// Function to process individual files and accumulate sums
void processFile(const char *filename, int *n, double *sumX, double *sumY, double *sumXY, double *sumX2) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }
    int x, y;
    while (fscanf(file, "%d,%d", &x, &y) == 2) {
        *sumX += x;         // Add to sum of x
        *sumY += y;         // Add to sum of y
        *sumXY += x * y;    // Add to sum of x * y
        *sumX2 += x * x;    // Add to sum of x^2
        (*n)++;             // Increment count of points
    }
    fclose(file);
}

// Function to calculate and print linear regression equation
void calculateAndPrintEquation(int n, double sumX, double sumY, double sumXY, double sumX2) {
    if (n == 0) {
        printf("No data available to compute regression.\n");
        return;
    }

    double denominator = n * sumX2 - sumX * sumX;
    if (denominator == 0) {
        printf("Error: Denominator is zero, cannot calculate regression.\n");
        return;
    }

    double a = (sumY * sumX2 - sumX * sumXY) / denominator; // Constant
    double b = (n * sumXY - sumX * sumY) / denominator;     // Slope

    // Print the combined equation
    printf("\nCombined Equation of the line: y = %.2fx + %.2f\n", b, a);

    // Ask user for an x value and calculate corresponding y
    double userX;
    printf("Enter a value for x to predict y: ");
    scanf("%lf", &userX);
    double userY = b * userX + a;
    printf("The corresponding value of y is: %.2f\n", userY);
}

int main() {
    // List of files to process
    const char *filenames[] = {"datasetLR1.txt", "datasetLR2.txt", "datasetLR3.txt", "datasetLR4.txt"};
    int numFiles = sizeof(filenames) / sizeof(filenames[0]);

    // Variables to store aggregated sums
    int n = 0;               // Total number of points
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

    // Process each file and accumulate sums
    for (int i = 0; i < numFiles; i++) {
        printf("Processing file: %s\n", filenames[i]);
        processFile(filenames[i], &n, &sumX, &sumY, &sumXY, &sumX2);
    }

    // Calculate and print the combined regression equation
    calculateAndPrintEquation(n, sumX, sumY, sumXY, sumX2);

    return 0;
}
