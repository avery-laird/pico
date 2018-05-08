//
// Created by avery on 11/04/18.
//

#include <stdlib.h>
#include <stdio.h>
#include "types.h"

/* some utilities for check the validity of a tree */

int NodesAreConnected(struct Tree *tree) {
    if (tree == NULL)
        return 1;
    if (tree->left != NULL) {
        if (tree->left->parent == tree) {
            return NodesAreConnected(tree->left);
        } else {
            return 0;
        }
    }
    if (tree->right != NULL) {
        if (tree->right->parent == tree) {
            return NodesAreConnected(tree->right);
        } else {
            return 0;
        }
    } else return 1;
}
