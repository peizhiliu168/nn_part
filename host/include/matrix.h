#ifndef MATRIX_H
#define MATRIX_H

typedef struct matrix {
    int rows;
    int cols;
    double** vals;
} matrix_t;

matrix_t* create_matrix(int rows, int cols);

void destroy_matrix(matrix_t* m);

#endif