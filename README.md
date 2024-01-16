# aatree 

A single header file implementation of the [AA tree](https://en.wikipedia.org/wiki/AA_tree) in C

The AA tree is a balanced binary search tree by Arne Andersson.
It was introduced in 1993 as an alternative to the
[AVL tree](https://en.wikipedia.org/wiki/AVL_tree) and 
[Red-black tree](https://en.wikipedia.org/wiki/Red–black_tree).
It's just as fast as the other trees but has a much simpler design.
See the [original paper](https://user.it.uu.se/~arnea/ps/simp.pdf) for details.

## Features

- No dependencies
- No allocations
- Easy to understand C code
- Tests suite and benchmarks included

## Usage

The only file that you need is `aat.h`.

Below is an example of generating an aat tree implementation using the prefix
`my_tree` and accepting the user-defined node `struct my_node`.

```C
#include <stdio.h>
#include "aat.h"

// Make a node that represents an item in the tree. This can be anything, as
// long as it has the necessary AAT_FIELDS.
struct my_node {
    // Include the aat fields here. They don't have to be strictly named 'left', 
    // 'right', and 'level', but they do need to match the same names as in the
    // AAT_IMPL below.
    AAT_FIELDS(struct my_node, left, right, level);
    // A key that is unique to every node item.
    int key;
    int value;
};

// Define a compare function that keeps the tree in order.
static int my_node_compare(struct my_node *a, struct my_node *b) {
    return a->key < b->key ? -1 : a->key > b->key;
}

// Generate the functions.
AAT_IMPL(my_tree, struct my_node, left, right, level, my_node_compare);

int main(void) {
    // Make a root for the tree.
    struct my_node *root = 0;

    // Add some items.
    my_tree_insert(&root, &(struct my_node){ .key = 5, .value = 1 });
    my_tree_insert(&root, &(struct my_node){ .key = 3, .value = 2 });
    my_tree_insert(&root, &(struct my_node){ .key = 8, .value = 3 });
    my_tree_insert(&root, &(struct my_node){ .key = 1, .value = 4 });
    my_tree_insert(&root, &(struct my_node){ .key = 6, .value = 5 });

    // Search for an item
    struct my_node *item = my_tree_search(&root, &(struct my_node){ .key = 3 });
    printf("key: %d, value: %d (FOUND)\n", item->key, item->value);

    // Delete an item in the tree
    item = my_tree_delete(&root, &(struct my_node){ .key = 6 });
    printf("key: %d, value: %d (DELETED)\n", item->key, item->value);

    // Iterate over all items in the tree.
    item = my_tree_first(&root);
    while (item) {
        printf("key: %d, value: %d (ITER)\n", item->key, item->value);
        item = my_tree_next(&root, item);
    }
}
// Output
// key: 3, value: 2 (FOUND)
// key: 6, value: 5 (DELETED)
// key: 1, value: 4 (ITER)
// key: 3, value: 2 (ITER)
// key: 5, value: 1 (ITER)
// key: 8, value: 3 (ITER)
```

## API

Using the `AAT_IMPL` from the example above, the following functions will 
become be available.

```C
struct my_node *my_tree_insert(struct my_node **root, struct my_node *item);
struct my_node *my_tree_delete(struct my_node **root, struct my_node *key);
struct my_node *my_tree_search(struct my_node **root, struct my_node *key);
struct my_node *my_tree_delete_first(struct my_node **root);
struct my_node *my_tree_delete_last(struct my_node **root);
struct my_node *my_tree_first(struct my_node **root);
struct my_node *my_tree_last(struct my_node **root);
struct my_node *my_tree_iter(struct my_node **root, struct my_node *key);
struct my_node *my_tree_prev(struct my_node **root, struct my_node *item);
struct my_node *my_tree_next(struct my_node **root, struct my_node *item);
```

## Running tests

Test the `aat.h` single header file.

```sh
$ cc test.c && ./a.out
```

Test the `aat-dev.c` development file.

```sh
$ cc -DAAT_DEV aat-dev.c test.c && ./a.out
```

## Running benchmarks

```sh
cc -O3 aat.c test.c && ./a.out bench
```

The following benchmarks were run on my 2021 Apple M1 Max using clang-17. 

```
SEED=1703937904
Running benchmarks...
insert:       1000000 items in 0.25 secs, 249.92 ns/op, 4001280/sec
search:       1000000 items in 0.25 secs, 248.41 ns/op, 4025538/sec
delete:       1000000 items in 0.44 secs, 444.44 ns/op, 2250043/sec
delete-first: 1000000 items in 0.09 secs, 86.08 ns/op, 11616426/sec
delete-last:  1000000 items in 0.11 secs, 106.76 ns/op, 9366716/sec
```
