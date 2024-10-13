/*
Student No.: 111550093
Student Name: I-TING, CHU
Email: itingchu1005@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page. 
*/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <queue>
#include <sys/time.h>
using namespace std;

const int MAX_THREADS = 8;
const int MAX_ELEMENTS = 1000000;

vector<int> input_array;
vector<vector<int>> sorted_subarrays(8);
sem_t done, job_mutex;

struct Job {
    int id;
    int start;
    int end;
    int chunk_size;
    bool is_merge; // True if it's a merge job, False if it's a bubble sort job
};

vector<int> job_done(16, 0);
queue<Job> job_queue;  // Change from vector to queue

void bubble_sort(vector<int>& arr, int start, int end) {
    for (int i = start; i < end; i++) {
        for (int j = start; j < end - (i - start); j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

void merge(vector<int>& arr, int start, int mid, int end) {
    vector<int> left(arr.begin() + start, arr.begin() + mid + 1);
    vector<int> right(arr.begin() + mid + 1, arr.begin() + end + 1);
    
    int i = 0, j = 0, k = start;
    
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            arr[k] = left[i];
            i++;
        } else {
            arr[k] = right[j];
            j++;
        }
        k++;
    }
    
    while (i < left.size()) {
        arr[k] = left[i];
        i++;
        k++;
    }
    
    while (j < right.size()) {
        arr[k] = right[j];
        j++;
        k++;
    }
}

void* worker_thread(void* arg) {
    while (true) {
        sem_wait(&job_mutex);        
        /* Critical section: Pop a job from job_queue */
        if (job_queue.empty()) {
            sem_post(&job_mutex);
            sem_post(&done);
            break;
        }
        
        Job job = job_queue.front();  // Get the front job
        job_queue.pop();  // Remove the front job
        /* Critical section: End */
        sem_post(&job_mutex);
        
        if (job.is_merge) {
            int mid = job.start * job.chunk_size + job.chunk_size / 2;
            merge(input_array, job.start * job.chunk_size, mid, job.end * job.chunk_size - 1);
        } else {
            bubble_sort(input_array, job.start * job.chunk_size, (job.end + 1) * job.chunk_size - 1);
        }
        
        sem_wait(&job_mutex);     
        job_done[job.id] = 1;
        int neighbor_id = job.id + (job.id & 2 ? -1 : 1);
        if (job_done[neighbor_id]) {
            int mergejob_id = job.id >> 1;
            Job merge_job = {mergejob_id, mergejob_id << 1, mergejob_id << 1 + 1, job.chunk_size, true};
            job_queue.push(merge_job);
        }
        sem_post(&job_mutex);
        sem_post(&done);
    }
    return NULL;
}

void handle_input() {
    // Read input from file
    ifstream input_file("input.txt");
    if (!input_file.is_open()) {
        cerr << "Error opening input file" << endl;
    }

    int num_elements;
    input_file >> num_elements;
    input_file.ignore(); // Ignore the space after the number

    input_array.resize(num_elements);
    for (int i = 0; i < num_elements; ++i) {
        input_file >> input_array[i];
    }

    input_file.close();
}

void handle_output(vector<int>& temp_array, int n){
    // Output sorted array to file
    ofstream output_file("output_" + to_string(n) + ".txt");
    for (int num : temp_array) {
        output_file << num << " ";
    }
    output_file.close();
}

int main() {
    
    handle_input();

    for (int n = 1; n <= MAX_THREADS; n++) {
        // Start timing
        struct timeval start, stop;
        gettimeofday(&start, 0);

        vector<int> temp_array = input_array; // Copy input array to temp_array for each n
        
        sem_init(&done, 0, 0);
        sem_init(&job_mutex, 0, 1);
        
        // Initialization: Append 8 bubble sort jobs to job_list
        int chunk_size = temp_array.size() / 8;
        Job newjob;
        for (int i = 0; i < 8; i++) {
            newjob = {i + 8, i, i + 1, chunk_size, false};
            job_queue.push(newjob);
        }
        
        // Create worker threads
        vector<pthread_t> threads(n);
        for (int i = 0; i < n; i++) {
            pthread_create(&threads[i], NULL, worker_thread, NULL);
        }
        
        // Wait for all threads to finish
        for (int i = 0; i < n; i++) {
            sem_wait(&done);
        }
        
        gettimeofday(&stop, 0);
        int sec = stop.tv_sec - start.tv_sec, usec = stop.tv_usec - start.tv_usec;
        
        handle_output(temp_array, n);
        
        printf("worker thread #%d, elapsed %.6f ms\n", n, (sec + (usec / 1000000.0)) * 1000.0);

        sem_destroy(&done);
        sem_destroy(&job_mutex);
    }
    
    return 0;
}
