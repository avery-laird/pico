#include <stdio.h>
#include <stdlib.h>

/*
 * No matter how we store the data, we must redraw the visible buffer
 * each time a change is made. This is unavoidable. So the key is to
 * make those redrawings as fast as possible
 */


struct Position {
    int start, length;
};

typedef struct Piece {
    int index;
    struct Position *position;
};

struct Piece *MakePiece(int index, int length) {
    struct Piece *NewPiece = malloc(sizeof(struct Piece));
    struct Position *NewPos = malloc(sizeof(struct Position));
    NewPos->start = 0;
    NewPos->length = length;

    NewPiece->index = index;
    NewPiece->position = NewPos;
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
    int size_left, size_right;
};

struct Tree *MakeNode(struct Piece *piece) {
    /* takes piece, returns node that points to piece */
    struct Tree *NewNode = malloc(sizeof(struct Tree));
    NewNode->piece = piece;
    NewNode->left = NewNode->right = NewNode->parent = NULL;
    NewNode->size_right = NewNode->size_right = 0;
    return NewNode;
}

struct Tree *BSTInsert(struct Tree *tree, struct Tree **inserted, struct Piece *piece) {
    if (tree == NULL) {
        tree = MakeNode(piece);
        *inserted = tree;
    }

    if (piece->index < tree->piece->index) {
        tree->left = BSTInsert(tree->left, inserted, piece);
        tree->left->parent = tree;
    }
    if (piece->index > tree->piece->index) {
        tree->right = BSTInsert(tree->right, inserted, piece);
        tree->right->parent = tree;
    }
    return tree;
}

void PropogateOffset(struct Tree *tree) {
    while (tree->parent != NULL) {
        if (tree->parent->left == tree)
            tree->parent->size_left = tree->piece->position->length
                                          + tree->size_right + tree->size_left;
        else
            tree->parent->size_right = tree->piece->position->start
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

struct Tree *Splay(struct Tree *tree, struct Tree *node) {
    /* three cases: zig, zig-zig, zig-zag. We check
     * for these cases starting at the last inserted
     * node, and move up the tree. */
    while (node->parent != NULL) { /* tree is root, nothing to do */
        if (node->parent->parent == NULL) {
            /* must be zig case:
             *           A          B
             *          / \        / \
             *         B  3  ==>  1  A
             *        / \           / \
             *       1  2          2  3
             * or vice versa
             * */
            struct Tree *parent = node;
            struct Tree *child = node->parent;
            switch (ZigSide(node)) { /* we can collapse this into a child pointer array later */
                case left:
                    /* tree is the left subtree */
                    child->left = parent->right;
                    if (child->left != NULL)
                        child->left->parent = child;

                    parent->parent = child->parent;

                    child->parent = parent;
                    parent->right = child;
                    break;
                case right:
                    /* tree is the right subtree */
                    child->right = parent->left;
                    if (child->right != NULL)
                        child->left->parent = child;

                    parent->parent = child->parent;

                    child->parent = parent;
                    parent->left = child;
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
    /* perform normal bst insert */
    tree = BSTInsert(tree, &inserted, piece);
    if (inserted->parent != NULL)
        inserted->piece->position->start = inserted->parent->piece->position->length + 1;
    else
        inserted->piece->position->start = 0;

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