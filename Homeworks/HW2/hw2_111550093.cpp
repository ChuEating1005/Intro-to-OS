/*
Student No.: 111550093
Student Name: I-TING, CHU
Email: itingchu1005@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page. 
*/
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdint>  // For uint32_t

using namespace std;

// Function to initialize matrices A and B
uint32_t** matrix_init(int dim) {
    uint32_t **matrix = new uint32_t*[dim];
    for (int i = 0; i < dim; i++) {
        matrix[i] = new uint32_t[dim];
    }
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            matrix[i][j] = i * dim + j;
        }
    }
    return matrix;
}

// Function to perform matrix multiplication for a submatrix
void multiply(uint32_t **A, uint32_t **B, uint32_t *C, int dim, int start_row, int end_row) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < dim; j++) {
            C[i * dim + j] = 0;  // Initialize C[i][j] to 0 before accumulation
            for (int k = 0; k < dim; k++) {
                C[i * dim + j] += A[i][k] * B[k][j];  // Perform matrix multiplication
            }
        }
    }
}

int main() {
    int dim;
    cout << "Input the matrix dimension: ";
    cin >> dim;

    // Initialize matrices A and B
    uint32_t **A = matrix_init(dim);
    uint32_t **B = matrix_init(dim);

    // Allocate shared memory for matrix C
    int shmid = shmget(IPC_PRIVATE, dim * dim * sizeof(uint32_t), IPC_CREAT | 0600);
    uint32_t *C = (uint32_t *)shmat(shmid, NULL, 0);

    // Loop to test the performance with different numbers of processes
    for (int process_num = 1; process_num <= 16; process_num++) {
        printf("Multiplying matrices using %d processes\n", process_num);

        // Start the timer
        struct timeval start, end;
        gettimeofday(&start, 0);

        int rows_per_process = dim / process_num;

        // Initialize matrix C
        for (int i = 0; i < dim * dim; i++) {
            C[i] = 0;
        }
        for (int p = 0; p < process_num; p++) {
            if (fork() == 0) {
                int start_row = p * rows_per_process;
                int end_row = (p == process_num - 1) ? dim : (p + 1) * rows_per_process;
                multiply(A, B, C, dim, start_row, end_row);
                exit(0);
            }
        }

        // Wait for all child processes to finish
        for (int p = 0; p < process_num; p++) {
            wait(NULL);
        }

        // End the timer
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        printf("Elapsed time: %f sec, ", sec + (usec / 1000000.0));

        // Calculate the checksum using 32-bit unsigned integers
        uint32_t checksum = 0;
        for (int i = 0; i < dim * dim; i++) {
            checksum += C[i];  // Allow overflow naturally
        }
        printf("Checksum: %u\n", checksum);  // Use %u for uint32_t
    }

    // Detach and remove shared memory
    shmdt(C);
    shmctl(shmid, IPC_RMID, NULL);

    // Free dynamically allocated memory for A and B
    for (int i = 0; i < dim; i++) {
        delete[] A[i];
        delete[] B[i];
    }
    delete[] A;
    delete[] B;

    return 0;
}
