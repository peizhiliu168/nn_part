#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <stdlib.h>
#include <matrix.h>

// simply create matrix initialized with zeros
matrix_t* create_matrix(int rows, int cols) {
    matrix_t* m = TEE_Malloc(sizeof(matrix_t), TEE_MALLOC_FILL_ZERO);
    if (!m){
        return NULL;
    }

    m->rows = rows;
    m->cols = cols;

    m->vals = TEE_Malloc(sizeof(double*) * m->rows, TEE_MALLOC_FILL_ZERO);
    if (!m->vals) {
        destroy_matrix(m);
        return NULL;
    }
    for (int i=0; i<m->rows; ++i) {
        double* row = TEE_Malloc(sizeof(double) * m->cols, TEE_MALLOC_FILL_ZERO);
        if (!row) {
            destroy_matrix(m);
            return NULL;
        }
        m->vals[i] = row;
    }

    return m;
}

// create identity square matrix with dim
matrix_t* create_matrix_identity(int dim) {
    matrix_t* m = create_matrix(dim, dim);
    if (!m) {
        return NULL;
    }

    for (int i=0; i<m->cols; ++i) {
        m->vals[i][i] = 1;
    }
    
    return m;
}

// create matrix initialized with values in range randomly
matrix_t* create_matrix_random(int rows, int cols, double start, double end) {
    if (end >= start) {
        return NULL;
    }

    matrix_t* m = create_matrix(rows, cols);
    if (!m) {
        return NULL;
    }

    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            m->vals[i][j] = ((double)rand())/((double)RAND_MAX) * (end - start) + start;
        }
    }

    return m;
}

// destroy matrix
void destroy_matrix(matrix_t* m) {
    if (!m) {
        return;
    }
    if (!m->vals) {
        goto L1;
    }

    for (int i=0; i<m->rows; ++i) {
        if (!m->vals[i]) {
            break;
        }
        TEE_Free(m->vals[i]);
    }
    TEE_Free(m->vals);
L1:
    TEE_Free(m);
}

// create new matrix and copy over old matrix
matrix_t* copy_matrix(matrix_t* m) {
    if (!m) {
        return NULL;
    }

    matrix_t* copy = create_matrix(m->rows, m->cols);
    if (!copy) {
        return NULL;
    }

    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            copy->vals[i][j] = m->vals[i][j];
        }
    }

    return copy;
}

// add two matrices together
int add_matrix(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        return -1;
    }

    if (m->rows != n->rows || m->cols != n->cols || m->cols != result->cols || m->rows != result->rows) {
        return -1;
    }

    for (int i=0; i<result->rows; ++i) {
        for (int j=0; j<result->cols; ++j) {
            result->vals[i][j] = m->vals[i][j] + n->vals[i][j];
        }
    }

    return 0;
}

// matrix multiplication m x n
int mult_matrix(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        return -1;
    }

    if (m->cols != n->rows || m->rows != result->rows || n->cols != result->cols) {
        return -1;
    }

    for (int i = 0; i < m->rows; ++i) {
      for (int j = 0; j < n->cols; ++j) {
         for (int k = 0; k < m->cols; ++k) {
            result->vals[i][j] += m->vals[i][k] * n->vals[k][j];
         }
      }
   }

    return 0;
}

// transpose matrix 
matrix_t* transpose_matrix(matrix_t* m) {
    matrix_t* transpose = create_matrix(m->cols, m->rows);
    if (!transpose) {
        return NULL;
    }

    for (int i=0; i<transpose->rows; ++i) {
        for (int j=0; j<transpose->cols; ++j) {
            transpose->vals[i][j] = m->vals[j][i];
        }
    }

    return transpose;
}

// given a function that takes in a single value and 
// returns a single value, apply that function to every
// element of the matrix
int apply_matrix(double (*apply)(double), matrix_t* m){
    if (!m) {
        return -1;
    }
    
    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            m->vals[i][j] = apply(m->vals[i][j]);
        }
    } 

    return 0;
}

