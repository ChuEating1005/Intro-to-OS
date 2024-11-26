/*
Student No.: 111550093
Student Name: I-TING, CHU
Email: itingchu1005@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page. 
*/

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <sys/time.h>
#define HASH_SIZE 131101
#define hash_func(val) (val % HASH_SIZE)
#define int uint64_t

using namespace std;

struct Instruction {
    bool is_write;
    int pageNum;
};
vector<Instruction> instructions;

void read_file(const char* filename) {
    FILE *fin = fopen(filename, "r");
    if (!fin) {
        printf("Error: Cannot open file %s\n", filename);
        exit(1);
    }

    char buf[20];
    while (fgets(buf, 20, fin)) {
        Instruction inst;
        inst.is_write = (buf[0] == 'W');
        string hexOffset(buf + 2);
        int offset = stoull(hexOffset, nullptr, 16);
        inst.pageNum = offset >> 12;
        instructions.push_back(inst);
    }
    fclose(fin);
}

class PageCache {
    protected:
        int32_t size;
        int32_t current_size = 0;
        int32_t hit = 0;
        int32_t miss = 0;
        int32_t write_back = 0;
    public:
        PageCache(int size) : size(size) {}
        void printLog() { 
            printf("%d\t%d\t%d\t\t%.10f\t\t%d\n", size, hit, miss, (double)miss / (hit + miss), write_back); 
        }
};

class LRUCache : public PageCache {
    private:
        struct Node {
            int pageNum;
            bool dirty;
            Node* prev;
            Node* next;
            Node(int pageNum = 0, Node* prev = nullptr, Node* next = nullptr, bool dirty = false): pageNum(pageNum), prev(prev), next(next), dirty(dirty) {}
        };
        list<Node*> hash_table[HASH_SIZE];
        Node *head, *tail;

        Node* lookup(int pageNum) {
            int hash_index = hash_func(pageNum);
            for (auto &i : hash_table[hash_index]) {
                if (i->pageNum == pageNum) return i;
            }
            return nullptr;
        }

        void replace(Node* cur, bool dirty) {
            if (cur == nullptr) return;
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;

            cur->prev = head;
            cur->next = head->next;
            head->next->prev = cur;
            head->next = cur;
            if (dirty) cur->dirty = true;
        }

        void insert_front(int pageNum, bool dirty) {
            Node *new_node = new Node(pageNum, head, head->next, dirty);;
            head->next->prev = new_node;
            head->next = new_node;
            hash_table[hash_func(pageNum)].push_back(new_node);
        }

        void remove_back() {
            Node *del_node = tail->prev;
            tail->prev = del_node->prev;
            del_node->prev->next = tail;

            hash_table[hash_func(del_node->pageNum)].remove(del_node);
            if (del_node->dirty) write_back++;
            delete del_node;
        }

    public:
        LRUCache(int size) : PageCache(size) {
            head = new Node(-1, nullptr, nullptr, false); // Dummy head
            tail = new Node(-1, nullptr, nullptr, false); // Dummy tail
            head->next = tail; 
            tail->prev = head;
        }

        ~LRUCache() {
            // clean up all nodes
            Node* current = head;
            while (current) {
                Node* nextNode = current->next;
                delete current;
                current = nextNode;
            }
        }

        void simulation() {
            for (const auto& inst : instructions) {
                Node* target = lookup(inst.pageNum);
                if (target == nullptr) {
                    miss++;
                    if (current_size == size) {
                        remove_back();
                        current_size--;
                    }
                    insert_front(inst.pageNum, inst.is_write);
                    current_size++;
                } else {
                    hit++;
                    replace(target, inst.is_write);
                }
            }
        }
};

class CFLRUCache : public PageCache {
    private:
        struct Node {
            int pageNum;
            bool dirty, inCF;
            Node* prev;
            Node* next;
            Node(int pageNum = 0, Node* prev = nullptr, Node* next = nullptr, bool dirty = false, bool inCF = false): pageNum(pageNum), prev(prev), next(next), dirty(dirty), inCF(inCF) {}
        };
        struct Region {
            int total_size;
            int cur_size = 0;
            Node *head, *tail;
            Region(int total_size = 0, int cur_size = 0, Node* head = nullptr, Node* tail = nullptr): total_size(total_size), cur_size(cur_size), head(head), tail(tail) {}
        };
        list<Node*> hash_table[HASH_SIZE];
        list<Node*> cleanLRU;
        Region working, CF;


        Node* lookup(int pageNum) {
            int hash_index = hash_func(pageNum);
            for (auto &i : hash_table[hash_index]) {
                if (i->pageNum == pageNum) return i;
            }
            return nullptr;
        }

        void replace(Node* cur, bool dirty) {
            if (cur == nullptr) return;
            if (cur->inCF) {
                if (!cur->dirty) cleanLRU.remove(cur);
                CF.cur_size--;
                working.cur_size++;
                cur->inCF = false;
            }
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;

            cur->prev = working.head;
            cur->next = working.head->next;
            working.head->next->prev = cur;
            working.head->next = cur;
            if (dirty) cur->dirty = true;
        }

        void migrate() {
            Node* migrate_node = working.tail->prev;
            migrate_node->prev->next = working.tail;
            working.tail->prev = migrate_node->prev;
            working.cur_size--;

            if (!migrate_node->dirty) {
                cleanLRU.push_front(migrate_node);
            }
            migrate_node->inCF = true;
            migrate_node->prev = CF.head;
            migrate_node->next = CF.head->next;
            CF.head->next->prev = migrate_node;
            CF.head->next = migrate_node;
            CF.cur_size++;
        }

        void insert_front(int pageNum, bool dirty) {
            Node *new_node = new Node(pageNum, working.head, working.head->next, dirty, false);;
            working.head->next->prev = new_node;
            working.head->next = new_node;
            working.cur_size++;
            hash_table[hash_func(pageNum)].push_front(new_node);
        }

        void remove() {
            Node *del_node;
            
            if (!cleanLRU.empty()) {
                // Clean First LRU
                del_node = cleanLRU.back();
                del_node->prev->next = del_node->next;
                del_node->next->prev = del_node->prev;
                cleanLRU.pop_back();
            } 
            else {
                // LRU
                del_node = CF.tail->prev;
                CF.tail->prev = del_node->prev;
                del_node->prev->next = CF.tail;
                write_back++;
            }
            CF.cur_size--;
            hash_table[hash_func(del_node->pageNum)].remove(del_node);
            delete del_node;
        }

    public:
        CFLRUCache(int size) : PageCache(size) {        
            Node* working_head = new Node(-1, nullptr, nullptr, false, false); // Dummy head
            Node* working_tail = new Node(-1, nullptr, nullptr, false, false); // Dummy tail
            working = Region(size * 3 / 4, 0, working_head, working_tail);
            working.head->next = working_tail; 
            working.tail->prev = working_head;

            Node* CF_head = new Node(-1, nullptr, nullptr, false, true); // Dummy head
            Node* CF_tail = new Node(-1, nullptr, nullptr, false, true); // Dummy tail
            CF = Region(size / 4, 0, CF_head, CF_tail);
            CF.head->next = CF_tail; 
            CF.tail->prev = CF_head;
        }

        ~CFLRUCache() {
            // clean up all nodes
            Node* current = working.head;
            while (current) {
                Node* nextNode = current->next;
                delete current;
                current = nextNode;
            }

            current = CF.head;
            while (current) {
                Node* nextNode = current->next;
                delete current;
                current = nextNode;
            }
        }

        void simulation() {
            for (const auto& inst : instructions) {
                Node* target = lookup(inst.pageNum);
                if (target == nullptr) {
                    miss++;
                    if (current_size == size) {
                        remove();
                        current_size--;
                    }
                    if (working.cur_size == working.total_size) {
                        migrate();
                    }
                    insert_front(inst.pageNum, inst.is_write);
                    current_size++;
                } else {
                    hit++;
                    if (working.cur_size == working.total_size && target->inCF) {
                        migrate();
                    }
                    replace(target, inst.is_write);
                }
            }
        }
};

int32_t main(int32_t argc, char* argv[]) {
    struct timeval start, end;
    // Read the trace file into a memory buffe
    read_file(argv[1]);
    printf("LRU policy:\nFrame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");
    gettimeofday(&start, 0);
    
    {
        LRUCache LRU_4096(4096);
        LRU_4096.simulation();
        LRU_4096.printLog();
    }
    {
        LRUCache LRU_8192(8192);
        LRU_8192.simulation();
        LRU_8192.printLog();
    }
    {
        LRUCache LRU_16384(16384);
        LRU_16384.simulation();
        LRU_16384.printLog();
    }
    {
        LRUCache LRU_32768(32768);
        LRU_32768.simulation();
        LRU_32768.printLog();
    }
    {
        LRUCache LRU_65536(65536);
        LRU_65536.simulation();
        LRU_65536.printLog();
    }

    gettimeofday(&end, 0);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f sec\n\n", elapsed_time);

    printf("CFLRU policy:\nFrame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");
    gettimeofday(&start, 0);
    {
        CFLRUCache CFLRU_4096(4096);
        CFLRU_4096.simulation();
        CFLRU_4096.printLog();
    }
    {
        CFLRUCache CFLRU_8192(8192);
        CFLRU_8192.simulation();
        CFLRU_8192.printLog();
    }
    {
        CFLRUCache CFLRU_16384(16384);
        CFLRU_16384.simulation();
        CFLRU_16384.printLog();
    }
    {
        CFLRUCache CFLRU_32768(32768);
        CFLRU_32768.simulation();
        CFLRU_32768.printLog();
    }
    {
        CFLRUCache CFLRU_65536(65536);
        CFLRU_65536.simulation();
        CFLRU_65536.printLog();
    }
    gettimeofday(&end, 0);
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f sec\n\n", elapsed_time);
    return 0;
}