// Copyright 2023 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.


#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "aat.h"

struct aat_node {
    AAT_FIELDS(struct aat_node, left, right, level);
    int key;
};

static int aat_compare(struct aat_node *a, struct aat_node *b) {
    return a->key < b->key ? -1 : a->key > b->key;
}

#ifdef AAT_DEV
AAT_DEF(extern, aat, struct aat_node)
#else
AAT_IMPL(aat, struct aat_node, left, right, level, aat_compare)
#endif

static void aat_valid0(struct aat_node *T, struct aat_node *P, int level, 
    int *index, int *last_key)
{
    assert(T->level == level);
    // 1) The level of every leaf node is one.
    if (!T->right && !T->left) {
        assert(T->level == 1);
    }
    // 2) The level of every left child is exactly one less than that of its parent.
    if (T->left) {
        assert(T->left->level == T->level-1);
    }
    // 3) The level of every right child is equal to or one less than that of its parent.
    if (T->right) {
        assert(T->right->level == T->level || T->right->level == T->level-1);
        // 4) The level of every right grandchild is strictly less than that of its grandparent.
        if (P) {
            assert(T->right->level < P->level);
        }
    }
    // 5) Every node of level greater than one has two children.
    if (T->level > 1) {
        assert(T->left && T->right);
    }
    // 6) Nodes are in order
    if (T->left) {
        aat_valid0(T->left, T, T->left->level, index, last_key);
    }
    if (*index > 0) {
        assert(T->key > *last_key);
    }
    *last_key = T->key;
    (*index)++;
    if (T->right) {
        aat_valid0(T->right, T, T->right->level, index, last_key);
    }
}

static void aat_valid(struct aat_node **root) {
    if (*root) {
        assert((*root)->level > 0);
        int index = 0;
        int last_key = 0;
        aat_valid0(*root, 0, (*root)->level, &index, &last_key);
    }
}

static void aat_print0(struct aat_node *T, unsigned long long int n) {
    unsigned long long int pn = n / 2;

    printf("node%llu [label = \"<f0> | <f1> ", n);
    printf("%d:%d", T->key, T->level);
    printf("|<f2> \"];\n");

    if (pn != 0) {
        if (n % 2 == 0) {
            printf("\"node%llu\":f0 -> \"node%llu\":f1;\n", pn, n);
        } else {
            printf("\"node%llu\":f2 -> \"node%llu\":f1;\n", pn, n);
        }
    }
    if (T->left) {
        aat_print0(T->left, 2 * n);
    }

    if (T->right) {
        aat_print0(T->right, 2 * n + 1);
    }

}

void aat_print(struct aat_node **root) {
    printf("digraph aa_tree {\n");
    printf("node [shape = record];\n");
    if (*root) {
        aat_print0(*root, 1);
    }
    printf("}\n");
}

static int64_t getnow(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
        fprintf(stderr, "clock_gettime: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return now.tv_sec * INT64_C(1000000000) + now.tv_nsec;
}


static void shuffle(void *array, size_t numels, size_t elsize) {
    if (numels < 2) return;
    char tmp[elsize];
    char *arr = array;
    for (size_t i = 0; i < numels - 1; i++) {
        int j = i + rand() / (RAND_MAX / (numels - i) + 1);
        memcpy(tmp, arr + j * elsize, elsize);
        memcpy(arr + j * elsize, arr + i * elsize, elsize);
        memcpy(arr + i * elsize, tmp, elsize);
    }
}

#define key_node(i) (&(struct aat_node){.key=(i)}) 

void bench() {
    int N = 1000000;
    struct aat_node *root = 0;
    struct aat_node *nodes = malloc(N*sizeof(struct aat_node));
    assert(nodes);
    memset(nodes, 0, N*sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        nodes[i].key = i;
    }
    shuffle(nodes, N, sizeof(struct aat_node));
    int64_t start = getnow();
    for (int i = 0; i < N; i++) {
        aat_insert(&root, &nodes[i]);
    }
    double elapsed = (getnow()-start)/1e9;
    fprintf(stderr, "insert:       %d items in %.2f secs, %.2f ns/op, %.0f/sec\n", 
        N, elapsed, elapsed*1e9/(double)N, (double)N/elapsed);

    int *keys = malloc(N*sizeof(int));
    assert(keys);
    for (int i = 0; i < N; i++) {
        keys[i] = i;
    }
    shuffle(keys, N, sizeof(int));
    start = getnow();
    for (int i = 0; i < N; i++) {
        assert(aat_search(&root, key_node(keys[i]))->key == keys[i]);
    }
    elapsed = (getnow()-start)/1e9;
    fprintf(stderr, "search:       %d items in %.2f secs, %.2f ns/op, %.0f/sec\n", 
        N, elapsed, elapsed*1e9/(double)N, (double)N/elapsed);

    shuffle(keys, N, sizeof(int));
    start = getnow();
    for (int i = 0; i < N; i++) {
        assert(aat_delete(&root, key_node(keys[i]))->key == keys[i]);
    }
    elapsed = (getnow()-start)/1e9;
    fprintf(stderr, "delete:       %d items in %.2f secs, %.2f ns/op, %.0f/sec\n", 
        N, elapsed, elapsed*1e9/(double)N, (double)N/elapsed);

    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        aat_insert(&root, &nodes[i]);
    }

    shuffle(keys, N, sizeof(int));
    start = getnow();
    for (int i = 0; i < N; i++) {
        aat_delete_first(&root);
    }
    elapsed = (getnow()-start)/1e9;
    fprintf(stderr, "delete-first: %d items in %.2f secs, %.2f ns/op, %.0f/sec\n", 
        N, elapsed, elapsed*1e9/(double)N, (double)N/elapsed);

    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        assert(!aat_insert(&root, &nodes[i]));
    }
    shuffle(keys, N, sizeof(int));
    start = getnow();
    for (int i = 0; i < N; i++) {
        assert(aat_delete_last(&root)->key == N-i-1);
    }
    elapsed = (getnow()-start)/1e9;
    fprintf(stderr, "delete-last:  %d items in %.2f secs, %.2f ns/op, %.0f/sec\n", 
        N, elapsed, elapsed*1e9/(double)N, (double)N/elapsed);


}

int main(int argc, char *argv[]) {
    int SEED = getenv("SEED") ? atoi(getenv("SEED")) : time(0);
    fprintf(stderr, "SEED=%d\n", SEED);
    srand(SEED);

    if (argc > 1 && strcmp(argv[1], "bench") == 0) {
        fprintf(stderr, "Running benchmarks...\n");
        bench();
        return 0;
    } else {
        fprintf(stderr, "For benchmarks provide the 'bench' argument.\n");
    }
    fprintf(stderr, "Running tests...\n");

    int N = 1000;
    struct aat_node *root = 0;
    struct aat_node *nodes = malloc(N*sizeof(struct aat_node));
    assert(nodes);
    memset(nodes, 0, N*sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        nodes[i].key = i;
    }
    shuffle(nodes, N, sizeof(struct aat_node));
    // int64_t start = getnow();
    for (int i = 0; i < N; i++) {
        assert(!aat_search(&root, &nodes[i]));
        assert(!aat_insert(&root, &nodes[i]));
        assert(aat_search(&root, &nodes[i]));
        assert(aat_search(&root, &nodes[i])->key == nodes[i].key);
        aat_valid(&root);
    }

    // double elapsed = (getnow()-start)/1e9;
    // fprintf(stderr, "insert: %d items in %.2f secs, %.2f ns/op, %.0f/sec\n", 
    //     N, elapsed, elapsed*1e9/(double)N, (double)N/elapsed);

    // sequentially check for every key
    for (int i = 0; i < N; i++) {
        assert(aat_search(&root, key_node(i))->key == i);
    }
    assert(!aat_search(&root, key_node(-1)));
    assert(!aat_search(&root, key_node(N)));

    // sequentially deleted all nodes
    for (int i = 0; i < N; i++) {
        assert(aat_delete(&root, key_node(i))->key == i);
        aat_valid(&root);
        assert(!aat_search(&root, key_node(i)));
        assert(!aat_delete(&root, key_node(i)));
        
    }

    // reinsert all
    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        assert(!aat_insert(&root, &nodes[i]));
        assert(aat_search(&root, &nodes[i]));
        assert(aat_search(&root, &nodes[i])->key == nodes[i].key);
    }

    // randomly delete all nodes
    for (int i = 0; i < N; i++) {
        assert(aat_delete(&root, &nodes[i])->key == nodes[i].key);
        aat_valid(&root);
        assert(!aat_search(&root, &nodes[i]));
        assert(!aat_delete(&root, &nodes[i]));
    }

    // reinsert all
    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        assert(!aat_insert(&root, &nodes[i]));
    }

    // delete_min all nodes
    for (int i = 0; i < N; i++) {
        assert(aat_first(&root)->key == i);
        assert(aat_delete_first(&root)->key == i);
        aat_valid(&root);
    }

    // reinsert all
    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        assert(!aat_insert(&root, &nodes[i]));
    }

    // delete_max all nodes
    for (int i = 0; i < N; i++) {
        assert(aat_last(&root)->key == N-i-1);
        assert(aat_delete_last(&root)->key == N-i-1);
        aat_valid(&root);
    }

    // reinsert all
    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        assert(!aat_insert(&root, &nodes[i]));
    }

    // delete half the nodes
    int *keys = malloc(N*sizeof(int));
    assert(keys);
    for (int i = 0; i < N; i++) {
        keys[i] = i;
    }
    shuffle(keys, N, sizeof(int));
    struct aat_node **deleted = malloc(N*sizeof(struct aat_node*));
    assert(deleted);
    for (int i = 0; i < N/2; i++) {
        deleted[i] = aat_delete(&root, key_node(keys[i]));
        assert(deleted[i]->key == keys[i]);
        assert(!aat_delete(&root, key_node(keys[i])));
        assert(!aat_search(&root, key_node(keys[i])));
        aat_valid(&root);
    }

    // shuffle the half and reinsert
    shuffle(deleted, N/2, sizeof(struct aat_node*));
    for (int i = 0; i < N/2; i++) {
        assert(!aat_search(&root, deleted[i]));
        assert(!aat_insert(&root, deleted[i]));
        assert(aat_search(&root, deleted[i])->key == deleted[i]->key);
        assert(aat_insert(&root, deleted[i]) == deleted[i]);
        aat_valid(&root);
    }

    // sequentially check for every key
    for (int i = 0; i < N; i++) {
        assert(aat_search(&root, key_node(i))->key == i);
    }
    assert(!aat_search(&root, key_node(-1)));
    assert(!aat_search(&root, key_node(N)));

    // iterators    
    root = 0;
    nodes = malloc(N*sizeof(struct aat_node));
    assert(nodes);
    memset(nodes, 0, N*sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        nodes[i].key = i*10;
    }
    shuffle(nodes, N, sizeof(struct aat_node));
    for (int i = 0; i < N; i++) {
        assert(!aat_insert(&root, &nodes[i]));
        assert(aat_search(&root, &nodes[i])->key == nodes[i].key);
        aat_valid(&root);
    }
    for (int i = -9; i < N*10; i++) {
        struct aat_node *iter = aat_iter(&root, key_node(i));
        if (i < 0) {
            assert(iter->key == 0);
        } else if (i > (N-1)*10) {
            assert(!iter);
        } else {
            assert(iter);
            int j = i % 10 == 0 ? i : i / 10 * 10 + 10;
            assert(iter->key == j);
            for (int j = iter->key+10; j < N*10; j += 10) {
                iter = aat_next(&root, iter);
                assert(iter);
                assert(iter->key == j);
            }
        }
    }

    struct aat_node *iter = aat_first(&root);
    assert(iter->key == 0);

    for (int i = 10; i < N*10; i += 10) {
        iter = aat_next(&root, iter);
        assert(iter->key == i);
    }

    iter = aat_last(&root);
    assert(iter->key == (N-1)*10);

    for (int i = (N-2)*10; i <= 0; i -= 10) {
        iter = aat_prev(&root, iter);
        assert(iter->key == i);
    }

    fprintf(stderr, "PASSED\n");

    return 0;
}
