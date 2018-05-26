#ifndef PIECETABLE_H
#define PIECETABLE_H

#include "types.h"
#include <stdio.h>

void print_to_file(double x,double y,FILE *p);

void FreeTree(struct Tree *tree);

void PrintInorder(struct Tree *tree, FILE *p);

void TraverseInorder(struct Tree *tree);

void RecursiveInorder(struct Tree *tree,unsigned long int *offset);

struct Tree *Insert(struct Tree *tree,struct Piece *piece,unsigned long int index);

int max(int a,int b);

struct Tree *Splay(struct Tree *tree,struct Tree *node);

enum casetype ZigZigOrZigZag(struct Tree *tree);

enum side ZigSide(struct Tree *tree);

struct Tree *BSTInsert(struct Tree *tree,struct Tree **inserted,struct Piece *piece,unsigned long int index);

unsigned long int node_offset(struct Tree *node);

unsigned long int node_size(struct Tree *node);

int is_split(unsigned long int offset,unsigned long int index,unsigned long int parent_length);

struct Tree *MakeNode(struct Piece *piece);

struct Piece *MakePiece(char *start,unsigned long int length);

void PrintTree(struct Tree *t);

struct Tree *dequeue(struct Queue *q);

void enqueue(struct Queue *q,struct Tree *node);

struct Queue *MakeQueue(int capacity);

void ClearLog(Logger *L);

void PrintLog(Logger *L);

void Log(Logger *L,int level,char *msg,...);

Logger *InitLogger();

static Logger *L;

#endif