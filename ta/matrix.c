#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <stdlib.h>
#include <matrix.h>
#include <assert.h>
#include <arm_neon.h>

// simply create matrix initialized with zeros
matrix_t* create_matrix(int rows, int cols) {
    matrix_t* m = TEE_Malloc(sizeof(matrix_t), TEE_MALLOC_FILL_ZERO);
    if (!m){
        assert(false);
    }
    // DMSG("create new matrix with %d rows and %d cols", rows, cols);
    m->rows = rows;
    m->cols = cols;

    m->vals = TEE_Malloc(sizeof(float*) * m->rows, TEE_MALLOC_FILL_ZERO);
    if (!m->vals) {
        destroy_matrix(m);
        assert(false);
    }
    for (int i=0; i<m->rows; ++i) {
        float* row = TEE_Malloc(sizeof(float) * m->cols, TEE_MALLOC_FILL_ZERO);
        if (!row) {
            destroy_matrix(m);
            assert(false);
        }
        m->vals[i] = row;
    }

    return m;
}

// create identity square matrix with dim
matrix_t* create_matrix_identity(int dim) {
    matrix_t* m = create_matrix(dim, dim);
    if (!m) {
        assert(false);
    }

    for (int i=0; i<m->cols; ++i) {
        m->vals[i][i] = 1;
    }
    
    return m;
}

// create matrix initialized with values in range randomly
matrix_t* create_matrix_random(int rows, int cols, float start, float end) {
    assert(end > start);

    matrix_t* m = create_matrix(rows, cols);
    if (!m) {
        assert(false);
    }

    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            float val = ((float)rand())/((float)RAND_MAX) * (end - start) + start;
            m->vals[i][j] = val;
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
        // for (int j=0; j<m->cols; ++j){
        //     DMSG("destory matrix value: %d\n", m->vals[i][j]);
        // }
        TEE_Free(m->vals[i]);
    }
    TEE_Free(m->vals);
L1:
    TEE_Free(m);
}

// create new matrix and copy over old matrix
matrix_t* copy_matrix(matrix_t* m) {
    if (!m) {
        assert(false);
    }

    matrix_t* copy = create_matrix(m->rows, m->cols);
    if (!copy) {
        assert(false);
    }

    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            copy->vals[i][j] = m->vals[i][j];
        }
    }

    return copy;
}

// copies contents in matrix m over to resulting matrix
void copy_over_matrix(matrix_t* m, matrix_t* result) { 
    assert(m != NULL);
    assert(m->rows == result->rows && m->cols == result->cols);

    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            result->vals[i][j] = m->vals[i][j];
        }
    }
}

// add two matrices together
void add_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        assert(false);
    }

    // if (m->rows != n->rows || m->cols != n->cols || m->cols != result->cols || m->rows != result->rows) {
    //     assert(false);
    // }

    for (int i=0; i<result->rows; ++i) {
        for (int j=0; j<result->cols; ++j) {
            result->vals[i][j] = m->vals[i % m->rows][j % m->cols] + n->vals[i % n->rows][j % n->cols];
        }
    }
}

// add two matrices together
void subtract_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        assert(false);
    }

    // if (m->rows != n->rows || m->cols != n->cols || m->cols != result->cols || m->rows != result->rows) {
    //     assert(false);
    // }

    for (int i=0; i<result->rows; ++i) {
        for (int j=0; j<result->cols; ++j) {
            result->vals[i][j] = m->vals[i % m->rows][j % m->cols] - n->vals[i % n->rows][j % n->cols];
        }
    }
}

// add two matrices together
void mult_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        assert(false);
    }

    // if (m->rows != n->rows || m->cols != n->cols || m->cols != result->cols || m->rows != result->rows) {
    //     assert(false);
    // }

    for (int i=0; i<result->rows; ++i) {
        for (int j=0; j<result->cols; ++j) {
            result->vals[i][j] = m->vals[i % m->rows][j % m->cols] * n->vals[i % n->rows][j % n->cols];
        }
    }
}

// add two matrices together
void div_matrix_element(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        assert(false);
    }

    // if (m->rows != n->rows || m->cols != n->cols || m->cols != result->cols || m->rows != result->rows) {
    //     assert(false);
    // }

    for (int i=0; i<result->rows; ++i) {
        for (int j=0; j<result->cols; ++j) {
            result->vals[i][j] = m->vals[i % m->rows][j % m->cols] / n->vals[i % n->rows][j % n->cols];
        }
    }
}

// matrix multiplication m x n
void mult_matrix(matrix_t* m, matrix_t* n, matrix_t* result) {
    if (!m || !n || !result) {
        assert(false);
    }

    if (m->cols != n->rows || m->rows != result->rows || n->cols != result->cols) {
        assert(false);
    }

    //matrix_t* temp = create_matrix(result->rows, result->cols);

    for (int i = 0; i < m->rows; ++i) {
      for (int j = 0; j < n->cols; ++j) {
         for (int k = 0; k < m->cols; ++k) {
            result->vals[i][j] += m->vals[i][k] * n->vals[k][j];
         }
      }
   }

   //copy_over_matrix(temp, result);
}

matrix_t* col_sum_matrix(matrix_t* m){

    //The result matrix should be 1 * width
    matrix_t* result = create_matrix(1, m->cols);

    // For every col in result, we traverse the original matrix 
    for (int j = 0; j < m->cols; ++j){
        for (int i = 0; i < m->rows; ++i){
            result->vals[0][j] += m->vals[i][j]; 
        }
    }
    return result;
}

matrix_t* row_sum_matrix(matrix_t* m){

    //The result matrix should be height * 1
    matrix_t* result = create_matrix(m->rows, 1);

    // For every row in result, we traverse the original matrix 
    for (int i = 0; i < m->rows; ++i){
        for (int j = 0; j < m->cols; ++j){
            result->vals[i][0] += m->vals[i][j]; 
        }
    }
    return result;
}

// transpose matrix 
matrix_t* transpose_matrix(matrix_t* m) {
    matrix_t* transpose = create_matrix(m->cols, m->rows);
    if (!transpose) {
        assert(false);
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
void apply_matrix(float (*apply)(float), matrix_t* m){
    if (!m) {
        assert(false);
    }
    
    for (int i=0; i<m->rows; ++i) {
        for (int j=0; j<m->cols; ++j) {
            if (m->vals[i][j] <= 0.0) {
                //DMSG("negative!");
            }
            //DMSG("applying value: %x", m->vals[i][j]);
            m->vals[i][j] = apply(m->vals[i][j]);
        }
    } 
}

// copies over a submatrix of a matrix, note starts are inclusive
// and ends are exclusive. Ends must be greater than starts
matrix_t* copy_submatrix(matrix_t* m, int start_row, int end_row, int start_col, int end_col) {
    assert(m != NULL);
    assert(start_row >= 0 && start_row < m->rows);
    assert(end_row > 0 && end_row <= m->rows);
    assert(start_col >= 0 && start_col < m->cols);
    assert(end_col > 0 && end_col <= m->cols);
    assert(end_col > start_col && end_row > start_row);

    matrix_t* copy = create_matrix(end_row - start_row, end_col - start_col);
    for (int i=0; i < copy->rows; ++i) {
        for (int j=0; j < copy->cols; ++j) {
            copy->vals[i][j] = m->vals[start_row + i][start_col + j];
        }
    }

    return copy;
}