#ifndef TA_MATRIX_H
#define TA_MATRIX_H

typedef struct matrix {
    int rows;
    int cols;
    float** __restrict vals;
} matrix_t;

matrix_t* create_matrix(int rows, int cols);

matrix_t* create_matrix_identity(int dim);

matrix_t* create_matrix_random(int rows, int cols, float start, float end);

void destroy_matrix(matrix_t* m);

matrix_t* copy_matrix(matrix_t* m);

void copy_over_matrix(matrix_t* m, matrix_t* result);

void add_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result);

void subtract_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result);

void mult_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result);

void div_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result);

void mult_matrix(matrix_t* m, matrix_t* n, matrix_t* result);

matrix_t* col_sum_matrix(matrix_t* m);

matrix_t* row_sum_matrix(matrix_t* m);

matrix_t* transpose_matrix(matrix_t* m);

void apply_matrix(float (*apply)(float), matrix_t* m);

matrix_t* copy_submatrix(matrix_t* m, int start_row, int end_row, int start_col, int end_col);

#endif