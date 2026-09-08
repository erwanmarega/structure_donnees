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

static DynamicArray *da_create(void) {
    DynamicArray *arr = malloc(sizeof(DynamicArray));
    if (!arr) { fprintf(stderr, "da_create: alloc failed\n"); exit(EXIT_FAILURE); }
    arr->data = malloc(sizeof(int) * DA_INITIAL_CAPACITY);
    if (!arr->data) { fprintf(stderr, "da_create: alloc failed\n"); exit(EXIT_FAILURE); }
    arr->size = 0;
    arr->capacity = DA_INITIAL_CAPACITY;
    return arr;
}

static void da_grow(DynamicArray *arr) {
    int new_capacity = arr->capacity * 2;
    int *new_data = realloc(arr->data, sizeof(int) * new_capacity);
    if (!new_data) { fprintf(stderr, "da_grow: alloc failed\n"); exit(EXIT_FAILURE); }
    arr->data = new_data;
    arr->capacity = new_capacity;
}

static void da_insert_front(DynamicArray *arr, int value) {
    if (arr->size == arr->capacity) da_grow(arr);
    for (int i = arr->size; i > 0; i--) arr->data[i] = arr->data[i - 1];
    arr->data[0] = value;
    arr->size++;
}

static void da_insert_back(DynamicArray *arr, int value) {
    if (arr->size == arr->capacity) da_grow(arr);
    arr->data[arr->size] = value;
    arr->size++;
}

static int da_get(const DynamicArray *arr, int index) {
    return arr->data[index];
}

static int da_find(const DynamicArray *arr, int value) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->data[i] == value) return i;
    }
    return -1;
}

static void da_remove_front(DynamicArray *arr) {
    if (arr->size == 0) return;
    for (int i = 0; i < arr->size - 1; i++) arr->data[i] = arr->data[i + 1];
    arr->size--;
}

static long da_sum(const DynamicArray *arr) {
    long total = 0;
    for (int i = 0; i < arr->size; i++) total += arr->data[i];
    return total;
}

static void da_free(DynamicArray *arr) {
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

static LinkedList *ll_create(void) {
    LinkedList *list = malloc(sizeof(LinkedList));
    if (!list) { fprintf(stderr, "ll_create: alloc failed\n"); exit(EXIT_FAILURE); }
    list->head = NULL;
    list->size = 0;
    return list;
}

static Node *ll_new_node(int value, Node *next) {
    Node *n = malloc(sizeof(Node));
    if (!n) { fprintf(stderr, "ll_new_node: alloc failed\n"); exit(EXIT_FAILURE); }
    n->value = value;
    n->next = next;
    return n;
}

static void ll_insert_front(LinkedList *list, int value) {
    list->head = ll_new_node(value, list->head);
    list->size++;
}

static void ll_insert_back(LinkedList *list, int value) {
    Node *node = ll_new_node(value, NULL);
    if (!list->head) {
        list->head = node;
    } else {
        Node *cur = list->head;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }
    list->size++;
}

static int ll_get(const LinkedList *list, int index) {
    Node *cur = list->head;
    for (int i = 0; i < index; i++) cur = cur->next;
    return cur->value;
}

static int ll_find(const LinkedList *list, int value) {
    Node *cur = list->head;
    int index = 0;
    while (cur) {
        if (cur->value == value) return index;
        cur = cur->next;
        index++;
    }
    return -1;
}

static void ll_remove_front(LinkedList *list) {
    if (!list->head) return;
    Node *old_head = list->head;
    list->head = old_head->next;
    free(old_head);
    list->size--;
}

static long ll_sum(const LinkedList *list) {
    long total = 0;
    for (Node *cur = list->head; cur; cur = cur->next) total += cur->value;
    return total;
}

static void ll_free(LinkedList *list) {
    if (!list) return;
    Node *cur = list->head;
    while (cur) { Node *next = cur->next; free(cur); cur = next; }
    free(list);
}

/* ============================================================
 * Hash Table (good vs bad hash function)
 * ============================================================ */

#define TABLE_SIZE 10007

typedef struct Entry {
    int value;
    struct Entry *next;
} Entry;

typedef int (*HashFn)(int);

typedef struct {
    Entry *buckets[TABLE_SIZE];
    int size;
    HashFn hash;
} HashTable;

static int hash_good(int value) {
    return (int) (((unsigned) value) % TABLE_SIZE);
}

static int hash_bad(int value) {
    (void) value;
    return 0;
}

static HashTable *ht_create(HashFn fn) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) { fprintf(stderr, "ht_create: alloc failed\n"); exit(EXIT_FAILURE); }
    for (int i = 0; i < TABLE_SIZE; i++) table->buckets[i] = NULL;
    table->size = 0;
    table->hash = fn;
    return table;
}

static void ht_insert(HashTable *table, int value) {
    int idx = table->hash(value);
    Entry *entry = malloc(sizeof(Entry));
    if (!entry) { fprintf(stderr, "ht_insert: alloc failed\n"); exit(EXIT_FAILURE); }
    entry->value = value;
    entry->next = table->buckets[idx];
    table->buckets[idx] = entry;
    table->size++;
}

static int ht_contains(const HashTable *table, int value) {
    int idx = table->hash(value);
    for (Entry *cur = table->buckets[idx]; cur; cur = cur->next) {
        if (cur->value == value) return 1;
    }
    return 0;
}

static void ht_free(HashTable *table) {
    if (!table) return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *cur = table->buckets[i];
        while (cur) { Entry *next = cur->next; free(cur); cur = next; }
    }
    free(table);
}

/* ============================================================
 * Timing helper
 * ============================================================ */

static double elapsed_seconds(struct timespec start, struct timespec end) {
    return (double) (end.tv_sec - start.tv_sec) +
           (double) (end.tv_nsec - start.tv_nsec) / 1e9;
}

#define TIME_START() clock_gettime(CLOCK_MONOTONIC, &t_start)
#define TIME_END()   clock_gettime(CLOCK_MONOTONIC, &t_end)

/* ============================================================
 * Benchmark: Dynamic Array vs Linked List
 * ============================================================ */

#define REPEAT_SMALL 2000
#define REPEAT_OP    500
#define REPEAT_SUM   20

static void bench_array_list(int n) {
    struct timespec t_start, t_end;

    DynamicArray *arr = da_create();
    LinkedList *list = ll_create();
    for (int i = 0; i < n; i++) {
        da_insert_back(arr, i);
        ll_insert_front(list, i);
    }

    int mid = n / 2;
    volatile long sink = 0;

    TIME_START();
    for (int r = 0; r < REPEAT_SMALL; r++) sink += da_get(arr, mid);
    TIME_END();
    double t_da_get = elapsed_seconds(t_start, t_end) / REPEAT_SMALL;

    TIME_START();
    for (int r = 0; r < REPEAT_SMALL; r++) sink += ll_get(list, mid);
    TIME_END();
    double t_ll_get = elapsed_seconds(t_start, t_end) / REPEAT_SMALL;

    int find_repeat = n >= 100000 ? 200 : REPEAT_SMALL;

    TIME_START();
    for (int r = 0; r < find_repeat; r++) sink += da_find(arr, -1);
    TIME_END();
    double t_da_find = elapsed_seconds(t_start, t_end) / find_repeat;

    TIME_START();
    for (int r = 0; r < find_repeat; r++) sink += ll_find(list, -1);
    TIME_END();
    double t_ll_find = elapsed_seconds(t_start, t_end) / find_repeat;

    TIME_START();
    for (int r = 0; r < REPEAT_OP; r++) {
        da_insert_front(arr, -2);
        sink += arr->size;
    }
    TIME_END();
    double t_da_insert_front = elapsed_seconds(t_start, t_end) / REPEAT_OP;
    for (int r = 0; r < REPEAT_OP; r++) da_remove_front(arr);

    TIME_START();
    for (int r = 0; r < REPEAT_OP; r++) {
        ll_insert_front(list, -2);
        sink += list->size;
    }
    TIME_END();
    double t_ll_insert_front = elapsed_seconds(t_start, t_end) / REPEAT_OP;
    for (int r = 0; r < REPEAT_OP; r++) ll_remove_front(list);

    TIME_START();
    for (int r = 0; r < REPEAT_OP; r++) {
        da_insert_back(arr, -3);
        da_remove_front(arr);
    }
    TIME_END();
    double t_da_insert_back = elapsed_seconds(t_start, t_end) / REPEAT_OP;

    TIME_START();
    for (int r = 0; r < REPEAT_OP; r++) {
        ll_insert_back(list, -3);
        ll_remove_front(list);
    }
    TIME_END();
    double t_ll_insert_back = elapsed_seconds(t_start, t_end) / REPEAT_OP;

    TIME_START();
    for (int r = 0; r < REPEAT_OP; r++) {
        int saved = da_get(arr, 0);
        da_remove_front(arr);
        da_insert_back(arr, saved);
    }
    TIME_END();
    double t_da_remove_front = elapsed_seconds(t_start, t_end) / REPEAT_OP;

    TIME_START();
    for (int r = 0; r < REPEAT_OP; r++) {
        int saved = ll_get(list, 0);
        ll_remove_front(list);
        ll_insert_back(list, saved);
    }
    TIME_END();
    double t_ll_remove_front = elapsed_seconds(t_start, t_end) / REPEAT_OP;

    TIME_START();
    for (int r = 0; r < REPEAT_SUM; r++) sink += da_sum(arr);
    TIME_END();
    double t_da_sum = elapsed_seconds(t_start, t_end) / REPEAT_SUM;

    TIME_START();
    for (int r = 0; r < REPEAT_SUM; r++) sink += ll_sum(list);
    TIME_END();
    double t_ll_sum = elapsed_seconds(t_start, t_end) / REPEAT_SUM;

    printf("N = %d\n", n);
    printf("  %-22s %-18s %-18s\n", "operation", "DynamicArray (s)", "LinkedList (s)");
    printf("  %-22s %-18.9f %-18.9f\n", "get(n/2)", t_da_get, t_ll_get);
    printf("  %-22s %-18.9f %-18.9f\n", "find(absent)", t_da_find, t_ll_find);
    printf("  %-22s %-18.9f %-18.9f\n", "insert_front", t_da_insert_front, t_ll_insert_front);
    printf("  %-22s %-18.9f %-18.9f\n", "insert_back", t_da_insert_back, t_ll_insert_back);
    printf("  %-22s %-18.9f %-18.9f\n", "remove_front", t_da_remove_front, t_ll_remove_front);
    printf("  %-22s %-18.9f %-18.9f\n", "full traversal (sum)", t_da_sum, t_ll_sum);
    printf("\n");

    (void) sink;
    da_free(arr);
    ll_free(list);
}

/* ============================================================
 * Experiment: hash_good vs hash_bad
 * ============================================================ */

static void bench_hash(const char *label, HashFn fn, int n, int lookup_repeat) {
    struct timespec t_start, t_end;
    HashTable *table = ht_create(fn);

    TIME_START();
    for (int i = 0; i < n; i++) ht_insert(table, i);
    TIME_END();
    double t_insert = elapsed_seconds(t_start, t_end);

    volatile long sink = 0;
    TIME_START();
    for (int r = 0; r < lookup_repeat; r++) {
        int target = r % n;
        sink += ht_contains(table, target);
    }
    TIME_END();
    double t_contains = elapsed_seconds(t_start, t_end) / lookup_repeat;

    printf("  %-12s N=%-9d insert(total)=%-14.6f contains(avg)=%-16.9f\n",
           label, n, t_insert, t_contains);

    (void) sink;
    ht_free(table);
}

int main(void) {
    int sizes[] = {1000, 10000, 100000, 1000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("=== Dynamic Array vs Linked List ===\n\n");
    for (int i = 0; i < num_sizes; i++) {
        bench_array_list(sizes[i]);
    }

    printf("=== hash_good vs hash_bad ===\n\n");
    for (int i = 0; i < num_sizes; i++) {
        int n = sizes[i];
        bench_hash("hash_good", hash_good, n, 2000);
        int bad_repeat = n >= 100000 ? 50 : 2000;
        bench_hash("hash_bad", hash_bad, n, bad_repeat);
        printf("\n");
    }

    return 0;
}
