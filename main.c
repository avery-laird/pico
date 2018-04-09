#include <stdio.h>
#include <stdlib.h>

/*
 * No matter how we store the data, we must redraw the visible buffer
 * each time a change is made. This is unavoidable. So the key is to
 * make those redrawings as fast as possible.
 *
 * 1. Insert node
 * 2. Splay up node
 */


typedef struct Piece {
    unsigned long int start, length;
};

struct Piece *MakePiece(unsigned long int start, unsigned long int length) {
    struct Piece *NewPiece = malloc(sizeof(NewPiece));
    NewPiece->start = start;
    NewPiece->length = length;
    return NewPiece;
}

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
    left, right, parent, serror
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

struct Tree *MakeNode(struct Piece *piece) {
    /* takes piece, returns node that points to piece */
    struct Tree *NewNode = malloc(sizeof(struct Tree));
    NewNode->piece = piece;
    NewNode->left = NewNode->right = NewNode->parent = NULL;
    NewNode->size_right = NewNode->size_right = 0;
    return NewNode;
}


int is_split(unsigned long int offset, unsigned long int index, unsigned long int parent_length) {
    /* takes offset and index of node to be inserted, returns 1
     * if a split is needed, 0 otherwise. */
    unsigned long int relative_left_bound = offset - parent_length;
    unsigned long int relative_right_bound = parent_length;
    unsigned long int relative_index = index - relative_left_bound;
    /*if ((offset - parent_length) < (index - (offset-parent_length)) && (index - (offset-parent_length)) < (offset + parent_length))*/
    if (relative_left_bound < relative_index && relative_index < relative_right_bound)
        return 1;
    else
        return 0;
}

unsigned long int node_size(struct Tree *node) {
    if (node == NULL)
        return 0; /* we don't consider strings of length 0 to be meaningful */
    else
        return node->size_left + node->size_right + node->piece->length;
}

unsigned long int node_offset(struct Tree *node) {
    if (node == NULL)
        return 0;
    else
        return node->size_left + node->piece->length;
}

struct Tree *BSTInsert(struct Tree *tree, struct Tree **inserted, struct Piece *piece, unsigned long int index) {
    /* interative BST insert, which also returns the address to the newly inserted node,
     * or NULL if the insert failed. Note that the function will still return the head to
     * the unchanged tree. Now we pass the index as a parameter, because once we use it
     * to sort the pieces, we don't need it anymore, and it's a waste to store.
     *
     * TODO: fix sizes
     */

    unsigned long int offset = 0;

    struct Tree *nodeptr = tree;
    if (nodeptr == NULL) {
        tree = MakeNode(piece);
        *inserted = tree;
        return tree;
    } else { /* iterate down the tree */
        offset += tree->size_left + tree->piece->length;
        while (1) {
            if (index <= offset) { /* if index is equal, we make it left child, because it prepends the existing piece */
                if (nodeptr->left != NULL) {
                    nodeptr = nodeptr->left;
                    /* if we're going into the left branch, offset is NOT additive */
                    offset = node_offset(nodeptr);
                    /* update offset only if child is not null */
                }
                else {
                    /* insert node as left child */
                    if (is_split(offset, index, nodeptr->piece->length)) {
                        /* Perform split. We know that the left child is definitely
                         * null, so we insert the linked list on that side. To keep
                         * the order correct, we insert like this:
                         *
                         *      last_half (nodeptr)
                         *            /
                         *        new_piece
                         *          /
                         *      first_half
                         *
                         * length_first = length_total - index_desired + index_current
                         * length_last = length_total - length_first = index_desired - index_current
                         */

                        /* 1. Save node's length, then update it */
                        unsigned long int total_length = nodeptr->piece->length;
                        /*unsigned long int last_half_length = offset - (index - 1);*/
                        unsigned long int last_half_length = offset - index;
                        nodeptr->piece->length = last_half_length;

                        /* 2. Insert new piece in the middle */
                        nodeptr->left = MakeNode(piece);
                        nodeptr->left->parent = nodeptr;

                        /* 3. Make a new piece to hold the last half of the split piece */
                        struct Tree *first_half = MakeNode(MakePiece( nodeptr->piece->start,
                                                                      total_length - last_half_length));

                        /* update last-half start index */
                        nodeptr->piece->start = first_half->piece->start + first_half->piece->length + 1;
                        nodeptr->left->left = first_half;
                        first_half->parent = nodeptr->left;

                        /* update subtree sizes */
                        nodeptr->left->size_left = node_size(nodeptr->left->left);
                        nodeptr->size_left = node_size(nodeptr->left);

                        *inserted = nodeptr->left;

                        return tree;
                    } else {
                        /* insert node normally */
                        nodeptr->left = MakeNode(piece);
                        nodeptr->left->parent = nodeptr;
                        *inserted = nodeptr->left;
                        return tree;
                    }
                }
            }
            if (index > offset) {
                if (nodeptr->right != NULL) {
                    nodeptr = nodeptr->right;
                    /* if we're going into the right subtree, offsets ARE additive */
                    offset += node_offset(nodeptr);
                    /* update offset only if child is not null */
                }
                else {
                    /* insert node as right child */
                    if (is_split(offset, index, nodeptr->piece->length)) {
                        /* Perform split. We know that the right child is definitely
                         * null, so we insert the linked list on that side. To keep
                         * the order correct, we insert like this:
                         *
                         * first_half (nodeptr)
                         *         \
                         *     new_piece
                         *          \
                         *      last_half
                         */

                        /* 1. Save node's length, then update it */
                        unsigned long int total_length = nodeptr->piece->length;
                        unsigned long int last_half_length = total_length - offset + index;

                        nodeptr->piece->length -= last_half_length;

                        /* 2. Insert new piece in the middle */
                        nodeptr->right = MakeNode(piece);
                        nodeptr->right->parent = nodeptr;

                        /* 3. Make a new piece to hold the last half of the split piece */
                        struct Tree *last_half = MakeNode(MakePiece( nodeptr->piece->start + nodeptr->piece->length + 1,
                                                                     last_half_length));

                        nodeptr->right->right= last_half;
                        last_half->parent = nodeptr->right;

                        /* update subtree sizes */
                        nodeptr->right->size_left = node_size(nodeptr->right->right);
                        nodeptr->size_right = node_size(nodeptr->right);

                        *inserted = nodeptr->right;

                        return tree;
                    } else {
                        /* insert node normally */
                        nodeptr->right = MakeNode(piece);
                        nodeptr->right->parent = nodeptr;
                        *inserted = nodeptr->right;
                        return tree;
                    }
                }
            }
        }
    }
}

void PropogateOffset(struct Tree *tree) {
    while (tree->parent != NULL) {
        if (tree->parent->left == tree)
            tree->parent->size_left = tree->piece->length + tree->size_right + tree->size_left;
        else
            tree->parent->size_right = tree->piece->length + tree->size_right + tree->size_left;
        tree = tree->parent;
    }
}

enum side ZigSide(struct Tree *tree) {
    if (tree->parent->right == tree)
        return right;
    else if (tree->parent->left == tree)
        return left;
    else
        return serror;
}

enum casetype ZigZigOrZigZag(struct Tree *tree) {
    if (tree->parent->left == tree && tree->parent->parent->left == tree->parent)
        return zigzigleft;
    if (tree->parent->right == tree && tree->parent->parent->right == tree->parent)
        return zigzigright;
    if (tree->parent->left == tree && tree->parent->parent->right == tree->parent)
        return zigzagleft;
    if (tree->parent->right == tree && tree->parent->parent->left == tree->parent)
        return zigzagright;
    else
        return cerror;
}


struct Tree *Splay(struct Tree *tree, struct Tree *node) {
    /* three cases: zig, zig-zig, zig-zag. We check
     * for these cases starting at the last inserted
     * node, and move up the tree. */

    while (node->parent != NULL) { /* tree is root, nothing to do */
        if (node->parent->parent == NULL) {
            /* must be zig case:
             *           C          P
             *          / \        / \
             *         P  3  ==>  1  C
             *        / \           / \
             *       1  2          2  3
             * or vice versa
             * */
            struct Tree *parent = node;
            struct Tree *child = node->parent;

            switch (ZigSide(node)) { /* we can collapse this into a child pointer array later */
                case left:
                    /* tree is the left subtree. */

                    /* 2 */
                    child->left = parent->right;
                    child->size_left = node_size(child->left);
                    if (child->left != NULL)
                        child->left->parent = child;

                    parent->parent = child->parent;

                    child->parent = parent;
                    parent->right = child;
                    parent->size_right = node_size(parent->right);

                    break;
                case right:
                    /* tree is the right subtree */
                    child->right = parent->left;
                    if (child->right != NULL)
                        child->right->parent = child;
                    child->size_right = node_size(child->right);

                    parent->parent = child->parent;

                    child->parent = parent;
                    parent->left = child;

                    parent->size_left = node_size(parent->left);
                    break;
                case serror:
                    printf("Error! Could not determine zig side: %ld", node->size_left + node->piece->length);
                    break;
            }
            tree = parent;
        }
        else if (node->parent->parent != NULL) {
            /* either zig-zig or zig-zag */
            struct Tree *parent = node;
            struct Tree *child = node->parent;
            struct Tree *grandchild = node->parent->parent;
            struct Tree *link = grandchild->parent;

            /* we can assume the top three pointers are all NOT null
             * link MAY BE null */
            switch (ZigZigOrZigZag(node)) {
                case zigzigleft:
                    /*       G         P
                     *      / \       / \
                     *     C  4      1   C
                     *    / \    =>     / \
                     *   P  3          2   G
                     *  / \               / \
                     * 1  2              3  4
                     */
                    /* change grandchild pointers */
                    grandchild->left = child->right;
                    grandchild->size_left = node_size(grandchild->left);
                    if (grandchild->left != NULL)
                        grandchild->left->parent = grandchild;

                    grandchild->parent = child;

                    /* change child pointers */
                    child->left = parent->right;
                    child->size_left = node_size(child->left);
                    if (child->left != NULL)
                        child->left->parent = child;
                    child->right = grandchild;
                    child->size_right = node_size(child->right);

                    /* change parent pointers */
                    parent->right = child;
                    parent->size_right = node_size(parent->right);

                    child->parent = parent;
                    parent->parent = link;
                    if (parent->parent == NULL)
                        tree = parent;

                    break;
                case zigzagright:
                    /*      G           P
                     *     / \        /   \
                     *    C  4       C    G
                     *   / \    =>  / \  / \
                     *  1   P      1  2  3 4
                     *    /  \
                     *   2   3
                     */
                    /* 2 */
                    grandchild->left = parent->right;
                    grandchild->size_left = node_size(grandchild->left);
                    if (grandchild->left != NULL)
                        grandchild->left->parent = grandchild;

                    /* 3 */
                    child->right = parent->left;
                    child->size_right = node_size(child->right);
                    if (child->right != NULL)
                        child->right->parent = child;

                    parent->left = child;
                    parent->right = grandchild;
                    parent->size_left = node_size(parent->left);
                    parent->size_right = node_size(parent->right);

                    grandchild->parent = parent;
                    child->parent = parent;
                    parent->parent = link;

                    if (parent->parent == NULL)
                        tree = parent;

                    break;
                case zigzigright:
                    /*   G               P
                     *  / \             / \
                     * 1   C           C   4
                     *    / \    =>   / \
                     *   2   P       G   3
                     *      / \     / \
                     *     3  4    1  2
                     */
                    /* change grandchild pointers */
                    grandchild->right = child->left;
                    grandchild->size_right = node_size(grandchild->right);
                    if (grandchild->right != NULL)
                        grandchild->right->parent = grandchild;
                    grandchild->parent = child;

                    /* change child pointers */
                    child->right = parent->left;
                    child->size_right = node_size(child->right);
                    if (child->right != NULL)
                        child->right->parent = child;

                    child->left = grandchild;
                    child->size_left = node_size(child->left);

                    /* change parent pointers */
                    parent->left = child;
                    parent->size_left = node_size(parent->left);

                    child->parent = parent;
                    parent->parent = link;
                    if (parent->parent == NULL)
                        tree = parent;

                    break;
                case zigzagleft:
                    /*   G              P
                     *  / \           /   \
                     * 1   C         G    C
                     *    / \   =>  / \  / \
                     *   P  4      1  2 3  4
                     *  / \
                     * 2  3
                     */
                    /* 2 */
                    grandchild->right = parent->left;
                    if (grandchild->right != NULL)
                        grandchild->right->parent = grandchild;
                    grandchild->size_right = node_size(grandchild->right);

                    /* 3 */
                    child->left = parent->right;
                    if (child->left != NULL)
                        child->left->parent = child;
                    child->size_left = node_size(child->left);


                    parent->left = grandchild;
                    parent->right = child;
                    parent->size_left = node_size(parent->left);
                    parent->size_right = node_size(parent->right);

                    grandchild->parent = parent;
                    child->parent = parent;
                    parent->parent = link;

                    if (parent->parent == NULL)
                        tree = parent;

                    break;
                case cerror:
                    printf("Error! problem with splaying index %ld\n", node->size_left + node->piece->length);
                    return tree;
            }
        }
    }
    return tree;
}

int max(int a, int b) {
    return a > b ? a : b;
}

struct Tree *Insert(struct Tree *tree, struct Piece *piece, unsigned long int index) {
    struct Tree *inserted = NULL;
    unsigned long int offset = 0;
    /* perform normal bst insert */
    tree = BSTInsert(tree, &inserted, piece, index);

    /* splay up the node pointed to by inserted */
    tree = Splay(tree, inserted);

    return tree;
}

void RecursiveInorder(struct Tree *tree, unsigned long int *offset) {
    if (tree == NULL)
        return;
    RecursiveInorder(tree->left, offset);
    printf("%ld\t\t%ld\t\t%ld\n", *offset, tree->piece->length, tree->piece->start);
    *offset += tree->piece->length;
    RecursiveInorder(tree->right, offset);
}

void TraverseInorder(struct Tree *tree) {
    unsigned long int offset = 0;
    RecursiveInorder(tree, &offset);
    printf("%ld\n", offset);
}

int main() {
    printf("offset\tlength\tstart\n");

    struct Tree *test = NULL;

    test = Insert(test, MakePiece(0, 13), 0); /* string of length 13 @ index=0 */
    test = Insert(test, MakePiece(0, 1), 10); /* string of length 1 @ index=10 */
    test = Insert(test, MakePiece(0, 3), 5);  /* string of length 3 @ index=5 */

    printf("TEST\n");
    TraverseInorder(test);

    printf("PREPEND\n");
    struct Tree *prepend = NULL;
    prepend = Insert(prepend, MakePiece(0, 5), 0);
    prepend = Insert(prepend, MakePiece(0, 4), 0);
    TraverseInorder(prepend);

    printf("APPEND\n");
    struct Tree *append = NULL;
    append = Insert(append, MakePiece(0, 5), 0);
    append = Insert(append, MakePiece(0, 4), 6);
    TraverseInorder(append);

    printf("SPLIT - insert right\n");
    struct Tree *split_right = NULL;
    split_right = Insert(split_right, MakePiece(0, 5), 0);
    split_right = Insert(split_right, MakePiece(0, 4), 3);
    TraverseInorder(split_right);

    printf("SPLIT - insert left\n");
    struct Tree *split_left = NULL;
    split_left = Insert(split_left, MakePiece(0, 5), 0);
    split_left = Insert(split_left, MakePiece(0, 4), 3);
    TraverseInorder(split_left);

    return 0;
}