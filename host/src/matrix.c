#include <matrix.h>

#include <assert.h>
#include <stdlib.h>

// simply create matrix initialized with zeros
matrix_t* create_matrix(int rows, int cols) {
    matrix_t* m = malloc(sizeof(matrix_t));
    if (!m){
        assert(0);
    }

    m->rows = rows;
    m->cols = cols;

    m->vals = malloc(sizeof(double*) * m->rows);
    if (!m->vals) {
        destroy_matrix(m);
        assert(0);
    }
    for (int i=0; i<m->rows; ++i) {
        double* row = malloc(sizeof(double) * m->cols);
        if (!row) {
            destroy_matrix(m);
            assert(0);
        }
        m->vals[i] = row;
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
        // for (int j=0; j<m->cols; ++j){
        //     DMSG("destory matrix value: %d\n", m->vals[i][j]);
        // }
        free(m->vals[i]);
    }
    free(m->vals);
L1:
    free(m);
}
