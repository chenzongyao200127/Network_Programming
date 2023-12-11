#include <stdio.h>
#include <stdlib.h>

void printPascalsTriangle(int x);

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments are provided
    if (argc != 2) {
        printf("Usage: ./cgi-bin <number_of_rows>\n");
        return 1;
    }

    int x = atoi(argv[1]); // Convert the argument to an integer

    // Validate the input
    if (x <= 0) {
        printf("Please enter a positive integer for the number of rows.\n");
        return 1;
    }

    printPascalsTriangle(x);

    return 0;
}

void printPascalsTriangle(int x) {
    int arr[x][x]; // Create a 2D array to store the values of Pascal's Triangle

    // Initialize the first row and first column of every row to 1
    for (int line = 0; line < x; line++) {
        arr[line][0] = 1;
        arr[line][line] = 1;
    }

    // Calculate the values using Pascal's Triangle property
    for (int line = 2; line < x; line++) {
        for (int i = 1; i < line; i++) {
            arr[line][i] = arr[line-1][i-1] + arr[line-1][i];
        }
    }

    // Print Pascal's Triangle
    for (int line = 0; line < x; line++) {
        for (int i = 0; i <= line; i++) {
            printf("%d ", arr[line][i]);
        }
        printf("\n");
    }
}
