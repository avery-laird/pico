#include <stdio.h>
#include <stdlib.h>

/*
 * No matter how we store the data, we must redraw the visible buffer
 * each time a change is made. This is unavoidable. So the key is to
 * make those redrawings as fast as possible
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
     * if a split is needed, 0 otherwise */
    if (index > offset || index < offset + parent_length )
        return 1;
    else
        return 0;
}



struct Tree *BSTInsert(struct Tree *tree, struct Tree **inserted, struct Piece *piece, unsigned long int index) {
    /* interative BST insert, which also returns the address to the newly inserted node,
     * or NULL if the insert failed. Note that the function will still return the head to
     * the unchanged tree. Now we pass the index as a parameter, because once we use it
     * to sort the pieces, we don't need it anymore, and it's a waste to store. */

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
                    offset += nodeptr->left->size_left + nodeptr->left->piece->length;
                    /* update offset only if child is not null */
                }
                else {
                    /* insert node as left child */
                    if (is_split(offset, index, nodeptr->piece->length)) {
                        /* Perform split. We know that the left child is definitely
                         * null, so we insert the linked list on that side. */

                        /* 1. Save node's length, then update it */
                        unsigned long int old_length = nodeptr->piece->length;
                        nodeptr->piece->length = old_length - (index - offset);

                        /* 2. Insert new piece in the middle */
                        nodeptr->left = MakeNode(piece);
                        nodeptr->left->parent = nodeptr;

                        /* 3. Make a new piece to hold the last half of the split piece */
                        struct Tree *last_half = MakeNode(MakePiece( nodeptr->piece->start + nodeptr->piece->length,
                                                                     old_length - nodeptr->piece->length));
                        nodeptr->left->left = last_half;
                        last_half->parent = nodeptr->left->left;

                        *inserted = nodeptr->left;

                        return tree;
                    } else {
                        /* insert node normally */
                        nodeptr->left = MakeNode(piece);
                        nodeptr->left->parent = nodeptr;
                        return tree;
                    }
                }
            }
            if (index > offset) {
                if (nodeptr->right != NULL) {
                    nodeptr = nodeptr->right;
                    offset += nodeptr->right->size_left + nodeptr->right->piece->length;
                    /* update offset only if child is not null */
                }
                else {
                    /* insert node as right child */
                    if (is_split(offset, index, nodeptr->piece->length)) {
                        /* Perform split. We know that the right child is definitely
                         * null, so we insert the linked list on that side. */

                        /* 1. Save node's length, then update it */
                        unsigned long int old_length = nodeptr->piece->length;
                        nodeptr->piece->length = old_length - (index - offset);

                        /* 2. Insert new piece in the middle */
                        nodeptr->right = MakeNode(piece);
                        nodeptr->right->parent = nodeptr;

                        /* 3. Make a new piece to hold the last half of the split piece */
                        struct Tree *last_half = MakeNode(MakePiece( nodeptr->piece->start + nodeptr->piece->length,
                                                                     old_length - nodeptr->piece->length));
                        nodeptr->right->right = last_half;
                        last_half->parent = nodeptr->right->right;

                        *inserted = nodeptr->right;

                        return tree;
                    } else {
                        /* insert node normally */
                        nodeptr->right = MakeNode(piece);
                        nodeptr->right->parent = nodeptr;
                        return tree;
                    }
                }
            }
        }
    }
}

/*
struct Tree *BSTInsert(struct Tree *tree, struct Tree **inserted, unsigned long int *offset, struct Piece *piece) {

    if (tree == NULL) {
        tree = MakeNode(piece);
        *inserted = tree;
        return tree;
    }

    (*offset) += tree->piece->position->length + tree->size_left;

    if (piece->index < *offset) {
        tree->left = BSTInsert(tree->left, inserted, offset, piece);
        tree->left->parent = tree;
        tree->size_left += tree->left->piece->position->length;
    }
    if (piece->index > *offset) {
        tree->right = BSTInsert(tree->right, inserted, offset, piece);
        tree->right->parent = tree;
        tree->size_right += tree->right->piece->position->length;
    }
    return tree;
}
*/

void PropogateOffset(struct Tree *tree) {
    while (tree->parent != NULL) {
        if (tree->parent->left == tree)
            tree->parent->size_left = tree->piece->position->length
                                          + tree->size_right + tree->size_left;
        else
            tree->parent->size_right = tree->piece->position->length
                                           + tree->size_right + tree->size_left;
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

int size(struct Tree *tree) {
    if (tree == NULL)
        return 0;
    else
        return tree->piece->position->length;
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
            int preserve_left = parent->size_left;
            int preserve_right = parent->size_right;
            switch (ZigSide(node)) { /* we can collapse this into a child pointer array later */
                case left:
                    /* tree is the left subtree */
                    child->left = parent->right;
                    if (child->left != NULL)
                        child->left->parent = child;

                    parent->parent = child->parent;

                    child->parent = parent;
                    parent->right = child;

                    child->size_left = preserve_right;
                    PropogateOffset(child);
                    break;
                case right:
                    /* tree is the right subtree */
                    child->right = parent->left;
                    if (child->right != NULL)
                        child->left->parent = child;

                    parent->parent = child->parent;

                    child->parent = parent;
                    parent->left = child;

                    child->size_right = preserve_left;

                    PropogateOffset(child);
                    break;
                case serror:
                    printf("Error! Could not determine zig side: %d", node->piece->index);
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
            int preserve_left = parent->size_left;
            int preserve_right = parent->size_right;
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
                    if (grandchild->left != NULL)
                        grandchild->left->parent = grandchild;
                    grandchild->parent = child;

                    /* change child pointers */
                    child->left = parent->right;
                    if (child->left != NULL)
                        child->left->parent = child;
                    child->right = grandchild;

                    /* change parent pointers */
                    parent->right = child;
                    child->parent = parent;
                    parent->parent = link;
                    if (parent->parent == NULL)
                        tree = parent;

                    grandchild->size_left = preserve_left;
                    grandchild->size_right = preserve_right;
                    PropogateOffset(grandchild);

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
                    if (grandchild->left != NULL)
                        grandchild->left->parent = grandchild;
                    /* 3 */
                    child->right = parent->left;
                    if (child->right != NULL)
                        child->right->parent = child;

                    parent->left = child;
                    parent->right = grandchild;

                    grandchild->parent = parent;
                    child->parent = parent;
                    parent->parent = link;

                    if (parent->parent == NULL)
                        tree = parent;

                    grandchild->size_left = size(grandchild->left);
                    child->size_right = size(child->right);

                    PropogateOffset(grandchild);
                    PropogateOffset(child);
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
                    if (grandchild->right != NULL)
                        grandchild->right->parent = grandchild;
                    grandchild->parent = child;

                    /* change child pointers */
                    child->right = parent->left;
                    if (child->right != NULL)
                        child->right->parent = child;
                    child->left = grandchild;

                    /* change parent pointers */
                    parent->left = child;
                    child->parent = parent;
                    parent->parent = link;
                    if (parent->parent == NULL)
                        tree = parent;

                    grandchild->size_left = preserve_left;
                    grandchild->size_right = preserve_right;
                    PropogateOffset(grandchild);
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

                    grandchild->size_right = parent->size_left;

                    /* 3 */
                    child->left = parent->right;
                    if (child->left != NULL)
                        child->left->parent = child;

                    child->size_left = parent->size_right;

                    parent->left = grandchild;
                    parent->right = child;

                    grandchild->parent = parent;
                    child->parent = parent;
                    parent->parent = link;

                    if (parent->parent == NULL)
                        tree = parent;

                    grandchild->size_right = size(grandchild->right);
                    child->size_left = size(child->left);

                    PropogateOffset(grandchild);
                    PropogateOffset(child);

                    break;
                case cerror:
                    printf("Error! problem with splaying index %d\n", node->piece->index);
                    break;
            }
        }
    }
    return tree;
}

int max(int a, int b) {
    return a > b ? a : b;
}

struct Tree *FixTree(struct Tree *tree, struct Tree *node) {
    /* check if the newly inserted node
     * overlaps with the parent node */
    if ( node == tree) return tree;
    if ( node->piece->index < ( node->parent->piece->index +
            node->parent->piece->position->length) ) {

        /* keep track of n1's previous length */
        int old_length = node->parent->piece->position->length;
        /* truncate n1 */
        node->parent->piece->position->length = node->piece->index - 1;

        int new_index = node->piece->index + node->piece->position->length + 1;
        struct Tree *NewNode = MakeNode(MakePiece(new_index, old_length - (node->piece->index - 1)));
        NewNode->piece->position->start = node->parent->piece->position->length + 1;

        NewNode->parent = node;
        node->right = NewNode;

        /* propagate buffer offset */
        PropogateOffset(NewNode);

        return tree;
    }
}

struct Tree *Insert(struct Tree *tree, struct Piece *piece) {
    struct Tree *inserted = NULL;
    unsigned long int offset = 0;
    /* perform normal bst insert */
    tree = BSTInsert(tree, &inserted, &offset, piece);

    /* add new node if needed */
    tree = FixTree(tree, inserted);

    /* splay up the node pointed to by inserted */
    tree = Splay(tree, inserted);

    return tree;
}

void TraverseInorder(struct Tree *tree) {
    if (tree == NULL)
        return;
    TraverseInorder(tree->left);
    printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\n", tree->piece->index, tree->piece->position->start,
           tree->piece->position->length, tree->size_left, tree->size_right);
    TraverseInorder(tree->right);
}

int main() {
    struct Tree *test = NULL;

    test = Insert(test, MakePiece(0, 13));
    test = Insert(test, MakePiece(10, 1));
    test = Insert(test, MakePiece(5, 3));

    printf("index\tstart\tlength\n");
    TraverseInorder(test);

    return 0;
}