#include <stdio.h>
#include "lodepng.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// Global variables for image processing
int ***imagePixels;       // 3D array to store RGB values
unsigned char *imageData; // Raw image data
int blurKernelSize;       // Size of blur kernel

// Structure to hold image segment information for threading
struct ImageSegment
{
    int startHeight; // Starting row for this segment
    int endHeight;   // Ending row for this segment
    int totalHeight; // Total image height
    int totalWidth;  // Total image width
};

/**
 * Thread function to apply blur effect to an image segment
 * @param ptr Pointer to ImageSegment structure containing segment bounds
 */
void *applyBlurEffect(void *ptr)
{
    struct ImageSegment *segment = (struct ImageSegment *)ptr;
    int startRow = segment->startHeight;
    int endRow = segment->endHeight;
    int height = segment->totalHeight;
    int width = segment->totalWidth;
    int kernelRadius = blurKernelSize / 2;

    // Process each pixel in the segment
    for (int row = startRow; row <= endRow; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int sumRed = 0, sumGreen = 0, sumBlue = 0;
            int pixelCount = 0;

            // Apply kernel to current pixel
            for (int kernelRow = -kernelRadius; kernelRow <= kernelRadius; kernelRow++)
            {
                for (int kernelCol = -kernelRadius; kernelCol <= kernelRadius; kernelCol++)
                {
                    int currentRow = row + kernelRow;
                    int currentCol = col + kernelCol;

                    // Check if pixel is within image bounds
                    if (currentRow >= 0 && currentRow < height && currentCol >= 0 && currentCol < width)
                    {
                        sumRed += imagePixels[currentRow][currentCol][0];
                        sumGreen += imagePixels[currentRow][currentCol][1];
                        sumBlue += imagePixels[currentRow][currentCol][2];
                        pixelCount++;
                    }
                }
            }

            // Calculate average color values and update image data
            imageData[4 * width * row + 4 * col + 0] = sumRed / pixelCount;
            imageData[4 * width * row + 4 * col + 1] = sumGreen / pixelCount;
            imageData[4 * width * row + 4 * col + 2] = sumBlue / pixelCount;
        }
    }
    pthread_exit(0);
}

/**
 * Check if a file exists
 * @param filename Name of file to check
 * @return 1 if file exists, 0 otherwise
 */
int isFileExists(const char *filename)
{
    return access(filename, F_OK) != -1;
}

/**
 * Check if a string contains only digits
 * @param str String to check
 * @return 1 if string contains only digits, 0 otherwise
 */
int isNumeric(const char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (!isdigit(str[i]))
        {
            return 0;
        }
    }
    return 1;
}

int main()
{
    unsigned int decodeError, imageWidth, imageHeight;
    int i, j, k, threadCount;
    char inputFilename[50], outputFilename[50], threadInput[10], kernelInput[10];

    // Get and validate input image filename
    while (1)
    {
        printf("Enter input image (with .png extension): ");
        scanf("%s", inputFilename);
        if (!isFileExists(inputFilename))
        {
            printf("Error: The file does not exist. Please try again.\n");
        }
        else
        {
            break;
        }
    }

    // Decode input image
    decodeError = lodepng_decode32_file(&imageData, &imageWidth, &imageHeight, inputFilename);
    if (decodeError)
    {
        printf("Error decoding image %d: %s\n", decodeError, lodepng_error_text(decodeError));
        return 1;
    }

    // Get and validate thread count
    while (1)
    {
        printf("Enter number of threads to use: ");
        scanf("%s", threadInput);
        if (!isNumeric(threadInput))
        {
            printf("Error: Please enter a valid integer for the number of threads.\n");
        }
        else
        {
            threadCount = atoi(threadInput);
            if (threadCount <= 0)
            {
                printf("Error: Number of threads must be a positive integer.\n");
            }
            else if (threadCount > imageHeight)
            {
                threadCount = imageHeight;
                printf("Adjusted number of threads to %d (height of the image).\n", threadCount);
                break;
            }
            else
            {
                break;
            }
        }
    }

    // Get and validate kernel size
    while (1)
    {
        printf("Enter an odd kernel size (e.g., 3, 5, 7): ");
        scanf("%s", kernelInput);
        if (!isNumeric(kernelInput))
        {
            printf("Error: Kernel size must be a positive integer.\n");
        }
        else
        {
            blurKernelSize = atoi(kernelInput);
            if (blurKernelSize <= 0)
            {
                printf("Error: Kernel size must be a positive integer.\n");
            }
            else if (blurKernelSize % 2 == 0)
            {
                printf("Error: Kernel size must be an odd number.\n");
            }
            else
            {
                break;
            }
        }
    }

    // Allocate and initialize 3D array for image data
    imagePixels = (int ***)malloc(imageHeight * sizeof(int **));
    for (i = 0; i < imageHeight; i++)
    {
        imagePixels[i] = (int **)malloc(imageWidth * sizeof(int *));
        for (j = 0; j < imageWidth; j++)
        {
            imagePixels[i][j] = (int *)malloc(3 * sizeof(int));
            for (k = 0; k < 3; k++)
            {
                imagePixels[i][j][k] = imageData[4 * imageWidth * i + 4 * j + k];
            }
        }
    }

    // Calculate work distribution for threads
    int rowsPerThread = imageHeight / threadCount;
    int extraRows = imageHeight % threadCount;
    int currentRow = 0;

    // Create and launch threads
    struct ImageSegment segments[threadCount];
    pthread_t threadHandles[threadCount];

    for (i = 0; i < threadCount; i++)
    {
        segments[i].startHeight = currentRow;
        segments[i].endHeight = currentRow + rowsPerThread - 1;
        if (extraRows > 0)
        {
            segments[i].endHeight++;
            extraRows--;
        }
        segments[i].totalHeight = imageHeight;
        segments[i].totalWidth = imageWidth;
        pthread_create(&threadHandles[i], NULL, applyBlurEffect, &segments[i]);
        currentRow = segments[i].endHeight + 1;
    }

    // Wait for all threads to complete
    for (i = 0; i < threadCount; i++)
    {
        pthread_join(threadHandles[i], NULL);
    }

    // Get and validate output filename
    while (1)
    {
        printf("Enter output image name (with .png extension): ");
        scanf("%s", outputFilename);
        if (strstr(outputFilename, ".png") == NULL)
        {
            printf("Error: The output file must have a .png extension.\n");
        }
        else
        {
            break;
        }
    }

    // Encode and save output image
    unsigned char *pngOutput;
    size_t pngSize;
    lodepng_encode32(&pngOutput, &pngSize, imageData, imageWidth, imageHeight);
    lodepng_save_file(pngOutput, pngSize, outputFilename);

    // Free allocated memory
    for (i = 0; i < imageHeight; i++)
    {
        for (j = 0; j < imageWidth; j++)
        {
            free(imagePixels[i][j]);
        }
        free(imagePixels[i]);
    }
    free(imagePixels);
    free(imageData);
    free(pngOutput);

    printf("\nImage has been successfully blurred and saved as %s\n", outputFilename);
    return 0;
}