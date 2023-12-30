# aatree 

An implementation of the [AA tree](https://en.wikipedia.org/wiki/AA_tree) in C.

The AA tree is a balanced binary search tree by Arne Andersson.
It was introduced in 1993 as an alternative to the
[AVL tree](https://en.wikipedia.org/wiki/AVL_tree) and 
[Red-black tree](https://en.wikipedia.org/wiki/Red–black_tree).
It's just as fast as the other trees but has a much simpler design.
See the [original paper](https://user.it.uu.se/~arnea/ps/simp.pdf) for details.

This library is stable and tested but is not designed for general purpose use.
It's more or less a template for statically dropping into existing codebases.
If you want to use it you will probably have to modify the `struct aat_node` to
fit your needs.

## Features

- No dependencies
- No allocations
- Easy to understand C code
- Tests suite and benchmarks included

## API

```C
struct aat_node *aat_insert(struct aat_tree *tree, struct aat_node *item);
struct aat_node *aat_search(struct aat_tree *tree, struct aat_node *key);
struct aat_node *aat_delete(struct aat_tree *tree, struct aat_node *key);
struct aat_node *aat_delete_first(struct aat_tree *tree);
struct aat_node *aat_delete_last(struct aat_tree *tree);
struct aat_node *aat_first(struct aat_tree *tree);
struct aat_node *aat_last(struct aat_tree *tree);
struct aat_node *aat_iter(struct aat_tree *tree, struct aat_node *key);
struct aat_node *aat_prev(struct aat_tree *tree, struct aat_node *item);
struct aat_node *aat_next(struct aat_tree *tree, struct aat_node *item);
```

## Running tests

```sh
$ cc aat.c test.c && ./a.out
```

## Running benchmarks

```sh
cc -O3 aat.c test.c && ./a.out bench
```

The following benchmarks were run on my 2021 Apple M1 Max using clang-17. 

```
SEED=1703937904
Running benchmarks...
insert:       1000000 items in 0.23 secs, 231.73 ns/op, 4315330/sec
search:       1000000 items in 0.28 secs, 282.68 ns/op, 3537581/sec
delete:       1000000 items in 0.43 secs, 434.43 ns/op, 2301851/sec
delete-first: 1000000 items in 0.09 secs, 90.33 ns/op, 11070519/sec
delete-last:  1000000 items in 0.11 secs, 110.47 ns/op, 9052477/sec
```
