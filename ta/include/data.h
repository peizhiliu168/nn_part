#ifndef TA_DATA_H
#define TA_DATA_H

#include "matrix.h"

typedef struct data {
    // data matrices
    matrix_t* features; // size N x feature_size
    matrix_t* labels; // size N x classes

    // data info
    int N; // number of samples
    int feature_size;
    int classes;

    // tracks current data present
    int batch_size;
    int start;
    int end;
} data_t;

void init_data(matrix_t* features, matrix_t* labels, int batch_size);

void destroy_data(void);

matrix_t* wrap_data(double* data, int size, int rows, int cols);

extern data_t* data_loader;

#endif