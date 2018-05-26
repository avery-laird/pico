//
// Created by avery on 26/04/18.
//

#include "PieceTable.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


int main() {
    L = InitLogger();

    printf("offset\tlength\tstart\n");

    /*struct Tree *test = NULL;

    test = Insert(test, MakePiece(0, 13), 0); *//* string of length 13 @ index=0 *//*
    test = Insert(test, MakePiece(0, 1), 10); *//* string of length 1 @ index=10 *//*
    test = Insert(test, MakePiece(0, 3), 5);  *//* string of length 3 @ index=5 *//*

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
    TraverseInorder(split_left);*/

    struct Tree *profile = NULL;
    clock_t start, end;
    double total = 0, diff;

    unsigned long int target;

    srand(time(NULL));

    unsigned long test1[] = {0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned long current_max = 0;

#define RANGE 1000000
    //double data[RANGE];
    //double x[RANGE];
    FILE *fp = fopen("linear_avg.txt", "w");
    FILE *fp2 = fopen("linear_per_insert.txt", "w");
    for (unsigned long int i=0; i<RANGE; i++) {

        target = (unsigned long) (rand() % (current_max - 0 + 1));

        //printf("insert @ %ld\n", target);

        start = clock();
        profile = Insert(profile, MakePiece("T", 1), i);
        end = clock();

        current_max = target + 1 > current_max ? target + 1 : current_max;
        diff = ((double)(end-start))/CLOCKS_PER_SEC;
        total += diff;
        print_to_file((double)(i), total/i, fp);
        print_to_file((double)(i), diff, fp2);
    }
    //plot_array(x, data, RANGE);
    printf("%f\n", total/RANGE);
    FreeTree(profile);
    profile = NULL;
    fclose(fp);

    return 0;
}