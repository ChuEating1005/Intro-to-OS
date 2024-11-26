/*
Student No.: 111550093
Student Name: I-TING, CHU
Email: itingchu1005@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page. 
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEMORY_POOL_SIZE 20000 // Define the size of the memory pool
#define HEADER_SIZE 32 // Define the size of the block header
#define LEVELS 11 // Number of levels in the free list

struct block {
    uint32_t size; // 4 bytes
    uint32_t free; // 4 bytes
    struct block *prev_free; // 8 bytes
    struct block *next_free; // 8 bytes
    struct block *prev; // 8 bytes
};

struct block *free_lists[LEVELS+1] = {NULL}; // Array of free lists
struct block *first = NULL;
void *memory = NULL;

// Helper function to get the next block
struct block *next_block(struct block *current) {
    return current + current->size / 32 + 1;
}

// Helper function to get the previous block
int get_level(size_t n) {
    int level = 0;

    // Special case for numbers below 32
    if (n < 32) {
        return 0;
    }

    while (n >= 1) {
        n >>= 1; // Right shift to divide by 2
        level++;
    }
    
    return level - 5; // Subtract the offset
}

void add_to_free_list(struct block *blk) {
    int level = get_level(blk->size);
    blk->next_free = free_lists[level];
    
    // If the list is not empty, set the previous block
    if (free_lists[level]) { 
        free_lists[level]->prev_free = blk;
    }

    free_lists[level] = blk;
    blk->free = 1;
}

void remove_from_free_list(struct block *blk) {
    int level = get_level(blk->size);

    struct block *prev = blk->prev_free;
    struct block *next = blk->next_free;
    
    // Update the previous block
    if (prev) {
        prev->next_free = next;
    }
    else {
        free_lists[level] = next;
    }
    // Update the next block
    if (next) {
        next->prev_free = prev;
    }
    blk->prev_free = NULL;
    blk->next_free = NULL;
    blk->free = 0;
}

void split_block(struct block *blk, size_t alloc_size) {
    struct block *new_blk = blk + 1 + alloc_size / 32;
    struct block *next = next_block(blk);
    new_blk->size = blk->size - alloc_size - HEADER_SIZE;
    new_blk->free = 1;
    new_blk->prev_free = NULL;
    new_blk->next_free = NULL;
    new_blk->prev = blk;
    next->prev = new_blk;
    blk->size = alloc_size;
    add_to_free_list(new_blk);
}

void merge_blocks(struct block *blk1, struct block *blk2) {
    struct block *new_blk = blk1;
    struct block *next = next_block(blk2);
    new_blk->size += blk2->size + HEADER_SIZE;
    new_blk->prev_free = NULL;
    new_blk->next_free = NULL;
    if (next) {
        next->prev = new_blk;
    }
    add_to_free_list(new_blk);
}


void *malloc(size_t size) {
    static int initialized = 0; // Static variable to track initialization

    // Initialize memory pool on the first call
    if (!initialized) {
        memory = mmap(NULL, MEMORY_POOL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (memory == MAP_FAILED) {
            return NULL; 
        }
        // Initialize the first block
        first = memory; 
        first->size = MEMORY_POOL_SIZE - HEADER_SIZE;
        first->free = 1;
        first->prev = NULL;
        add_to_free_list(first);
        
        initialized = 1; // Mark as initialized
    }

    int level = get_level(size);
    size_t alloc_size = (size + 31) / 32 * 32; // align to 32 bytes
    struct block *best_fit = NULL;
    struct block *temp;

    if (size == 0) {
        size_t max = 0;
        int i = LEVELS - 1;
        while(i >= 0 && free_lists[i] == NULL) {
            i--;
        }
        if (i >= 0) {
            temp = free_lists[i];
            while(temp) {
                if (temp->size > max) 
                    max = temp->size;
                temp = temp->next_free;
                
            }
        }
        

        // output max free chunk size
        char output[100];
        sprintf(output, "Max Free Chunk Size = %ld\n", max);
        write(STDOUT_FILENO, output, strlen(output));
        
        munmap(memory, 20000);
        return NULL;
    }

    // Find the best fit block in the appropriate level
    for (int i = level; i < LEVELS; i++) {
        temp = free_lists[i];
        while (temp) {
            if (temp->size >= alloc_size && (best_fit == NULL || temp->size <= best_fit->size)) {
                best_fit = temp;
            }
            temp = temp->next_free;
            
        }
        if (best_fit) {
            break;
        }
    }


    // If no suitable block found, return NULL
    if (!best_fit) {
        return NULL;
    }
    else{
        // If the block is exactly the size of the allocation request, remove it from the free list
        if (best_fit->size - alloc_size == 0) {
            remove_from_free_list(best_fit);
        }
        else {
            remove_from_free_list(best_fit);
            split_block(best_fit, alloc_size);
           
        }
        return best_fit + 1;
    }   
}

void free(void *ptr) {
    struct block *blk = ptr - HEADER_SIZE;

    add_to_free_list(blk);

    struct block *prev = blk->prev;
    struct block *next = next_block(blk);

    if (prev && prev->free) {
        remove_from_free_list(prev);
        remove_from_free_list(blk);
        merge_blocks(prev, blk);
        blk = prev;
    }
    if (next && next->free) {
        remove_from_free_list(blk);
        remove_from_free_list(next);
        merge_blocks(blk, next);
    }
}