//
// Created by avery on 11/04/18.
//

#ifndef PIECETABLE_TYPES_H
#define PIECETABLE_TYPES_H

#include <stddef.h>

struct Message {
    int level;
    char *msg;
};

typedef struct MessageWrapper {
    struct Message *m;
    struct MessageWrapper *next;
} MessageWrapper;

typedef struct Logger {
    struct MessageWrapper *top;
} Logger;

typedef struct Piece {
    size_t length;
    char *start;
} Piece;

/* List of pieces */
struct Block {
    struct Piece *next;
};

/*
 * Store the pieces in a splay tree
 *
 * the key of the node is Tree->piece->index
 */

enum side {
    left, right, parent, serror, null
};

enum casetype {
    zigzigleft, zigzigright,
    zigzagleft, zigzagright,
    cerror
};

struct Tree {
    struct Piece *piece;
    struct Tree *left, *right, *parent;
    unsigned long int size_left, size_right;
};

struct Queue {
    int front, back, capacity, size;
    struct Tree **tree;
};


#endif //PIECETABLE_TYPES_H
