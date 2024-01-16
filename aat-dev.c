// https://github.com/tidwall/aat
//
// Copyright 2023 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.

// Do not use this in production. 
// This is only the development file for the aat.h single header file, which is
// all you need to generate a custom aat binary search tree.

////////////////////////////////////////////////////////////////////////////////
// AA-tree declarations
////////////////////////////////////////////////////////////////////////////////

struct aat_node {
    struct aat_node *left;
    struct aat_node *right;
    int level;
    int key;
};

struct aat_node *aat_insert(struct aat_node **root, struct aat_node *item);
struct aat_node *aat_delete(struct aat_node **root, struct aat_node *key);
struct aat_node *aat_search(struct aat_node **root, struct aat_node *key);
struct aat_node *aat_delete_first(struct aat_node **root);
struct aat_node *aat_delete_last(struct aat_node **root);
struct aat_node *aat_first(struct aat_node **root);
struct aat_node *aat_last(struct aat_node **root);
struct aat_node *aat_iter(struct aat_node **root, struct aat_node *key);
struct aat_node *aat_prev(struct aat_node **root, struct aat_node *item);
struct aat_node *aat_next(struct aat_node **root, struct aat_node *item);

////////////////////////////////////////////////////////////////////////////////
// AA-tree implementation
////////////////////////////////////////////////////////////////////////////////

static int aat_compare(struct aat_node *a, struct aat_node *b) {
    return a->key < b->key ? -1: a->key > b->key;
}

static void aat_clear(struct aat_node *node) {
    if (node) {
        node->left = 0;
        node->right = 0;
        node->level = 0;
    }
}

static struct aat_node *aat_skew(struct aat_node *node) {
    if (node && node->left && node->left->level == node->level) {
        struct aat_node *left = node->left;
        node->left = left->right;
        left->right = node;
        node = left;
    }
    return node;
}

static struct aat_node *aat_split(struct aat_node *node) {
    if (node && node->right && node->right->right && 
        node->right->right->level == node->level)
    {
        struct aat_node *right = node->right;
        node->right = right->left;
        right->left = node;
        right->level++;
        node = right;
    }
    return node;
}

static struct aat_node *aat_insert0(struct aat_node *node, 
    struct aat_node *item, struct aat_node **replaced)
{
    if (!node) {
        item->left = 0;
        item->right = 0;
        item->level = 1;
        node = item;
    } else {
        int cmp = aat_compare(item, node);
        if (cmp < 0) {
            node->left = aat_insert0(node->left, item, replaced);
        } else if (cmp > 0) {
            node->right = aat_insert0(node->right, item, replaced);
        } else {
            *replaced = node;
            item->left = node->left;
            item->right = node->right;
            item->level = node->level;
            node = item;
        }
    }
    node = aat_skew(node);
    node = aat_split(node);
    return node;
}

struct aat_node *aat_insert(struct aat_node **root, struct aat_node *item) {
    struct aat_node *replaced = 0;
    *root = aat_insert0(*root, item, &replaced);
    if (replaced != item) {
        aat_clear(replaced);
    }
    return replaced;
}

static struct aat_node *aat_decrease_level(struct aat_node *node) {
    if (node->left || node->right) {
        int level = 0;
        if (node->left && node->right) {
            if (node->left->level < node->right->level) {
                level = node->left->level;
            } else {
                level = node->right->level;
            }
        }
        level++;
        if (level < node->level) {
            node->level = level;
            if (node->right && level < node->right->level) {
                node->right->level = level;
            }
        }
    }
    return node;
}

static struct aat_node *aat_delete_fixup(struct aat_node *node) {
    node = aat_decrease_level(node);
    node = aat_skew(node);
    node->right = aat_skew(node->right);
    if (node->right && node->right->right) {
        node->right->right = aat_skew(node->right->right);
    }
    node = aat_split(node);
    node->right = aat_split(node->right);
    return node;
}

static struct aat_node *aat_delete_first0(struct aat_node *node,
    struct aat_node **deleted)
{
    if (node) {
        if (!node->left) {
            *deleted = node;
            if (node->right) {
                node = node->right;
            } else {
                node = 0;
            }
        } else {
            node->left = aat_delete_first0(node->left, deleted);
            node = aat_delete_fixup(node);
        }
    }
    return node;
}

static struct aat_node *aat_delete_last0(struct aat_node *node,
    struct aat_node **deleted)
{
    if (node) {
        if (!node->right) {
            *deleted = node;
            if (node->left) {
                node = node->left;
            } else {
                node = 0;
            }
        } else {
            node->right = aat_delete_last0(node->right, deleted);
            node = aat_delete_fixup(node);
        }
    }
    return node;
}

struct aat_node *aat_delete_first(struct aat_node **root) {
    struct aat_node *deleted = 0;
    *root = aat_delete_first0(*root, &deleted);
    aat_clear(deleted);
    return deleted;
}

struct aat_node *aat_delete_last(struct aat_node **root) {
    struct aat_node *deleted = 0;
    *root = aat_delete_last0(*root, &deleted);
    aat_clear(deleted);
    return deleted;
}

static struct aat_node *aat_delete0(struct aat_node *node, 
    struct aat_node *key, struct aat_node **deleted)
{
    if (node) {
        int cmp = aat_compare(key, node);
        if (cmp < 0) {
            node->left = aat_delete0(node->left, key, deleted);
        } else if (cmp > 0) {
            node->right = aat_delete0(node->right, key, deleted);
        } else {
            *deleted = node;
            if (!node->left && !node->right) {
                node = 0;
            } else {
                struct aat_node *leaf_deleted = 0;
                if (!node->left) {
                    node->right = aat_delete_first0(node->right, &leaf_deleted);
                } else {
                    node->left = aat_delete_last0(node->left, &leaf_deleted);
                }
                leaf_deleted->left = node->left;
                leaf_deleted->right = node->right;
                leaf_deleted->level = node->level;
                node = leaf_deleted;
            }
        }
        if (node) {
            node = aat_delete_fixup(node);
        }
    }
    return node;
}

struct aat_node *aat_delete(struct aat_node **root, struct aat_node *key) {
    struct aat_node *deleted = 0;
    *root = aat_delete0(*root, key, &deleted);
    aat_clear(deleted);
    return deleted;
}

struct aat_node *aat_search(struct aat_node **root, struct aat_node *key) {
    struct aat_node *found = 0;
    struct aat_node *node = *root;
    while (node) {
        int cmp = aat_compare(key, node);
        if (cmp < 0) {
            node = node->left;
        } else if (cmp > 0) {
            node = node->right;
        } else {
            found = node;
            node = 0;
        }
    }
    return found;
}

struct aat_node *aat_first(struct aat_node **root) {
    struct aat_node *node = *root;
    if (node) {
        while (node->left) {
            node = node->left;
        }
    }
    return node;
}

struct aat_node *aat_last(struct aat_node **root) {
    struct aat_node *node = *root;
    if (node) {
        while (node->right) {
            node = node->right;
        }
    }
    return node;
}

struct aat_node *aat_iter(struct aat_node **root, struct aat_node *key) {
    struct aat_node *found = 0;
    struct aat_node *node = *root;
    while (node) {
        int cmp = aat_compare(key, node);
        if (cmp < 0) {
            found = node;
            node = node->left;
        } else if (cmp > 0) {
            node = node->right;
        } else {
            found = node;
            node = 0;
        }
    }
    return found;
}

static struct aat_node *aat_parent(struct aat_node **root, 
    struct aat_node *item)
{
    struct aat_node *parent = 0;
    struct aat_node *node = *root;
    while (node) {
        int cmp = aat_compare(item, node);
        if (cmp < 0) {
            parent = node;
            node = node->left;
        } else if (cmp > 0) {
            parent = node;
            node = node->right;
        } else {
            node = 0;
        }
    }
    return parent;
}

struct aat_node *aat_next(struct aat_node **root, struct aat_node *node) {
    if (node) {
        if (node->right) {
            node = node->right;
            while (node->left) {
                node = node->left;
            }
        } else {
            struct aat_node *parent = aat_parent(root, node);
            while (parent && parent->left != node) {
                node = parent;
                parent = aat_parent(root, parent);
            }
            node = parent;
        }
    }
    return node;
}

struct aat_node *aat_prev(struct aat_node **root, struct aat_node *node) {
    if (node) {
        if (node->left) {
            node = node->left;
            while (node->right) {
                node = node->right;
            }
        } else {
            struct aat_node *parent = aat_parent(root, node);
            while (parent && parent->right != node) {
                node = parent;
                parent = aat_parent(root, parent);
            }
            node = parent;
        }
    }
    return node;
}
