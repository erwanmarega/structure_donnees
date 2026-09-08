#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ============================================================
 * Dynamic Array    
 * ============================================================ */


typedef struct {
    int *data;
    int size;
    int capacity;
} DynamicArray;

#define DA_INITIAL_CAPACITY 8

DynamicArray *da_create(void) {
    DynamicArray *arr = malloc(sizeof(DynamicArray));
    if (!arr) {
        fprintf(stderr, "da_create: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    arr->data = malloc(sizeof(int) * DA_INITIAL_CAPACITY);
    if (!arr->data) {
        fprintf(stderr, "da_create: allocation failed\n");
        free(arr);
        exit(EXIT_FAILURE);
    }
    arr->size = 0;
    arr->capacity = DA_INITIAL_CAPACITY;
    return arr;
}

static void da_grow(DynamicArray *arr) {
    int new_capacity = arr->capacity * 2;
    int *new_data = realloc(arr->data, sizeof(int) * new_capacity);
    if (!new_data) {
        fprintf(stderr, "da_grow: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    arr->data = new_data;
    arr->capacity = new_capacity;
}

void da_insert_front(DynamicArray *arr, int value) {
    if (arr->size == arr->capacity) {
        da_grow(arr);
    }
    for (int i = arr->size; i > 0; i--) {
        arr->data[i] = arr->data[i - 1];
    }
    arr->data[0] = value;
    arr->size++;
}

void da_insert_back(DynamicArray *arr, int value) {
    if (arr->size == arr->capacity) {
        da_grow(arr);
    }
    arr->data[arr->size] = value;
    arr->size++;
}

int da_get(const DynamicArray *arr, int index, int *ok) {
    if (index < 0 || index >= arr->size) {
        if (ok) *ok = 0;
        return 0;
    }
    if (ok) *ok = 1;
    return arr->data[index];
}

int da_find(const DynamicArray *arr, int value) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->data[i] == value) {
            return i;
        }
    }
    return -1;
}

void da_remove_front(DynamicArray *arr) {
    if (arr->size == 0) {
        return;
    }
    for (int i = 0; i < arr->size - 1; i++) {
        arr->data[i] = arr->data[i + 1];
    }
    arr->size--;
}

void da_free(DynamicArray *arr) {
    if (!arr) return;
    free(arr->data);
    free(arr);
}

/* ============================================================
 * Linked List
 * ============================================================ */

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    int size;
} LinkedList;

LinkedList *ll_create(void) {
    LinkedList *list = malloc(sizeof(LinkedList));
    if (!list) {
        fprintf(stderr, "ll_create: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

static Node *ll_new_node(int value, Node *next) {
    Node *n = malloc(sizeof(Node));
    if (!n) {
        fprintf(stderr, "ll_new_node: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    n->value = value;
    n->next = next;
    return n;
}

void ll_insert_front(LinkedList *list, int value) {
    list->head = ll_new_node(value, list->head);
    list->size++;
}

void ll_insert_back(LinkedList *list, int value) {
    Node *node = ll_new_node(value, NULL);
    if (!list->head) {
        list->head = node;
    } else {
        Node *cur = list->head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = node;
    }
    list->size++;
}

int ll_get(const LinkedList *list, int index, int *ok) {
    if (index < 0 || index >= list->size) {
        if (ok) *ok = 0;
        return 0;
    }
    Node *cur = list->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    if (ok) *ok = 1;
    return cur->value;
}

int ll_find(const LinkedList *list, int value) {
    Node *cur = list->head;
    int index = 0;
    while (cur) {
        if (cur->value == value) {
            return index;
        }
        cur = cur->next;
        index++;
    }
    return -1;
}

void ll_remove_front(LinkedList *list) {
    if (!list->head) {
        return;
    }
    Node *old_head = list->head;
    list->head = old_head->next;
    free(old_head);
    list->size--;
}

void ll_free(LinkedList *list) {
    if (!list) return;
    Node *cur = list->head;
    while (cur) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    free(list);
}

/* ============================================================
 * Hash Table (separate chaining)
 * ============================================================ */

#define TABLE_SIZE 10007

typedef struct Entry {
    int value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry *buckets[TABLE_SIZE];
    int size;
} HashTable;

HashTable *ht_create(void) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) {
        fprintf(stderr, "ht_create: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    table->size = 0;
    return table;
}

static unsigned hash_index(int value) {
    return ((unsigned) value) % TABLE_SIZE;
}

void hash_insert(HashTable *table, int value) {
    unsigned idx = hash_index(value);
    Entry *entry = malloc(sizeof(Entry));
    if (!entry) {
        fprintf(stderr, "hash_insert: allocation failed\n");
        exit(EXIT_FAILURE);
    }
    entry->value = value;
    entry->next = table->buckets[idx];
    table->buckets[idx] = entry;
    table->size++;
}

int hash_contains(const HashTable *table, int value) {
    unsigned idx = hash_index(value);
    Entry *cur = table->buckets[idx];
    while (cur) {
        if (cur->value == value) {
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

void hash_remove(HashTable *table, int value) {
    unsigned idx = hash_index(value);
    Entry *cur = table->buckets[idx];
    Entry *prev = NULL;
    while (cur) {
        if (cur->value == value) {
            if (prev) {
                prev->next = cur->next;
            } else {
                table->buckets[idx] = cur->next;
            }
            free(cur);
            table->size--;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void hash_free(HashTable *table) {
    if (!table) return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *cur = table->buckets[i];
        while (cur) {
            Entry *next = cur->next;
            free(cur);
            cur = next;
        }
    }
    free(table);
}

/* ============================================================
 * Benchmark
 * ============================================================ */

#define NUM_LOOKUPS 2000
#define SEED 42

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1e9;
}

static void bench_dynamic_array(int n) {
    DynamicArray *arr = da_create();

    double t0 = now_seconds();
    for (int i = 0; i < n; i++) {
        da_insert_back(arr, i);
    }
    double t_insert_back = now_seconds() - t0;

    t0 = now_seconds();
    volatile long checksum = 0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        int target = rand() % n;
        checksum += da_find(arr, target);
    }
    double t_find = now_seconds() - t0;

    int ok;
    t0 = now_seconds();
    checksum += da_get(arr, n / 2, &ok);
    double t_get = now_seconds() - t0;

    int drain_count = n < 20000 ? n : 20000;
    t0 = now_seconds();
    for (int i = 0; i < drain_count; i++) {
        da_remove_front(arr);
    }
    double t_remove_front = (now_seconds() - t0) / drain_count;

    printf("%-14s %-10d %-16.6f %-16.9f %-16.9f %-18.9f\n",
           "DynamicArray", n, t_insert_back,
           t_find / NUM_LOOKUPS, t_get, t_remove_front);

    da_free(arr);
    (void) checksum;
}

static void bench_linked_list(int n) {
    LinkedList *list = ll_create();

    double t0 = now_seconds();
    for (int i = 0; i < n; i++) {
        ll_insert_front(list, i); 
    }
    double t_insert_front = now_seconds() - t0;

    t0 = now_seconds();
    volatile long checksum = 0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        int target = rand() % n;
        checksum += ll_find(list, target);
    }
    double t_find = now_seconds() - t0;

    int ok;
    t0 = now_seconds();
    checksum += ll_get(list, n / 2, &ok);
    double t_get = now_seconds() - t0;

    int drain_count = n < 20000 ? n : 20000;
    t0 = now_seconds();
    for (int i = 0; i < drain_count; i++) {
        ll_remove_front(list);
    }
    double t_remove_front = (now_seconds() - t0) / drain_count;

    printf("%-14s %-10d %-16.6f %-16.9f %-16.9f %-18.9f\n",
           "LinkedList", n, t_insert_front,
           t_find / NUM_LOOKUPS, t_get, t_remove_front);

    ll_free(list);
    (void) checksum;
}

static void bench_hash_table(int n) {
    HashTable *table = ht_create();

    double t0 = now_seconds();
    for (int i = 0; i < n; i++) {
        hash_insert(table, i);
    }
    double t_insert = now_seconds() - t0;

    t0 = now_seconds();
    volatile long checksum = 0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        int target = rand() % n;
        checksum += hash_contains(table, target);
    }
    double t_contains = now_seconds() - t0;

    printf("%-14s %-10d %-16.6f %-16.9f %-16s %-18s\n",
           "HashTable", n, t_insert,
           t_contains / NUM_LOOKUPS, "N/A", "N/A");

    hash_free(table);
    (void) checksum;
}

int main(void) {
    srand(SEED);

    int sizes[] = {1000, 10000, 100000, 1000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("%-14s %-10s %-16s %-16s %-16s %-18s\n",
           "Structure", "N", "insert (total s)", "find (avg s/op)",
           "get[N/2] (s)", "remove_front (avg s)");
    printf("insert column: insert_back for DynamicArray, "
           "insert_front for LinkedList, hash_insert for HashTable\n");
    printf("----------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_sizes; i++) {
        bench_dynamic_array(sizes[i]);
        bench_linked_list(sizes[i]);
        bench_hash_table(sizes[i]);
        printf("\n");
    }

    return 0;
}
