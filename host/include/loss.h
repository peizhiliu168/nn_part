#ifndef TA_LOSS_H
#define TA_LOSS_H

#include "matrix.h"

double multiclass_softmax(matrix_t* pred, matrix_t* label);

void softmax_single(double* arr, int size);

void softmax(matrix_t* m);

double mean_cross_entropy_softmax(matrix_t* logits, matrix_t* labels);

matrix_t* d_mean_cross_entropy_softmax(matrix_t* logits, matrix_t* labels);

#endif