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
#include <utility>
#include <fstream>
#include <algorithm>
#include <queue>
#include <sys/time.h>
using namespace std;

const int MAX_THREADS = 8;
const int MAX_ELEMENTS = 1000000;
const int MAX_JOBS = 16; // Increased from 20 to accommodate all possible jobs

vector<int> input_array;
sem_t thread_done, job_mutex;

struct Job {
    int id;
    int start;
    int end;
    int chunk_size;
    bool is_merge; // True if it's a merge job, False if it's a bubble sort job
};

vector<int> job_done(MAX_JOBS, 0);
vector<int> job_chunk(MAX_JOBS, 0);
vector<pair<int, int>> job_range(MAX_JOBS, {0, 0});
queue<Job> job_queue;  // Change from vector to queue

void bubble_sort(vector<int>& arr, int start, int end) {
    for (int i = start; i < end + 1; i++) {
        bool swapped = false;
        for (int j = start; j < end - (i - start); j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

void merge(vector<int>& arr, int id) {
    int start = job_range[id*2].first;
    int end = job_range[id*2+1].second;
    int mid = job_range[id*2].second;

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
        Job job;
        bool has_job = false;

        sem_wait(&job_mutex);
        if (!job_queue.empty()) {
            job = job_queue.front();
            job_queue.pop();
            has_job = true;
        }
        sem_post(&job_mutex);

        if (!has_job) {
            break; // Exit the loop if no more jobs
        }

        if (job.is_merge) {
            merge(input_array, job.id);
        } else {
            bubble_sort(input_array, job.start, job.end);
        }

        job_done[job.id] = 1;
        

        sem_wait(&job_mutex);
        int neighbor_id = (job.id & 1) ? job.id - 1 : job.id + 1;
        if (job.id > 1 && job_done[neighbor_id]) {
            int parent_id = job.id >> 1;
            int start = min(job.start, job_range[neighbor_id].first);
            int end = max(job.end, job_range[neighbor_id].second);
            Job merge_job = {parent_id, start, end, job_chunk[job.id] + job_chunk[neighbor_id], true};
            job_queue.push(merge_job);
            job_range[parent_id] = {start, end};
        }
        sem_post(&job_mutex);
    }
    sem_post(&thread_done); // Signal that this thread is done
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

    vector<int> temp_array = input_array;
    for (int n = 1; n <= MAX_THREADS; n++) {
        input_array = temp_array;
        sem_init(&thread_done, 0, 0);
        sem_init(&job_mutex, 0, 1);

        // Reset job_done array
        fill(job_done.begin(), job_done.end(), 0);

        // Initialization: Append 8 bubble sort jobs to job_queue
        int chunk_size = input_array.size() / 8;
        int remainder = input_array.size() % 8;

        for (int i = 0; i < 8; i++) {
            job_chunk[i+8] = chunk_size + (i < remainder ? 1 : 0);
        }
        
        Job newjob;
        for (int i = 0; i < 8; i++) {
            if (i == 0)
                newjob = {i + 8, 0, job_chunk[8] - 1, job_chunk[8], false};
            else
                newjob = {i + 8, newjob.end + 1, newjob.end + job_chunk[i+8], job_chunk[i+8], false};
            job_range[i + 8] = {newjob.start, newjob.end};
            job_queue.push(newjob);
        }

        // Start timing
        struct timeval start, stop;
        gettimeofday(&start, 0);

        // Create worker threads
        vector<pthread_t> threads(n);
        for (int i = 0; i < n; i++) {
            pthread_create(&threads[i], NULL, worker_thread, NULL);
        }
        
        // Wait for all threads to finish
        for (int i = 0; i < n; i++) {
            sem_wait(&thread_done);
        }

        for (int i = 0; i < n; i++) {
            pthread_join(threads[i], NULL);
        }
        
        gettimeofday(&stop, 0);
        int sec = stop.tv_sec - start.tv_sec, usec = stop.tv_usec - start.tv_usec;
        
        handle_output(input_array, n);
        
        printf("worker thread #%d, elapsed %.6f ms\n", n, (sec + (usec / 1000000.0)) * 1000.0);

        sem_destroy(&thread_done);
        sem_destroy(&job_mutex);
    }
    
    return 0;
}
