#ifndef TA_MATRIX_H
#define TA_MATRIX_H

typedef struct matrix {
    int rows;
    int cols;
    double** vals;
} matrix_t;

matrix_t* create_matrix(int rows, int cols);

matrix_t* create_matrix_identity(int dim);

matrix_t* create_matrix_random(int rows, int cols, double start, double end);

void destroy_matrix(matrix_t* m);

matrix_t* copy_matrix(matrix_t* m);

int add_matrix(matrix_t* m, matrix_t* n);

int mult_matrix(matrix_t* m, matrix_t* n);

matrix_t* transpose_matrix(matrix_t* m);

int apply_matrix(double (*apply)(double), matrix_t* m);

#endif