#include <loss.h>
#include <math_TA.h>
#include <assert.h>

// label is a 1xn matrix 
double multiclass_softmax(matrix_t* pred, matrix_t* label) {
    double loss = 0;
    for (int i=0; i<pred->rows; ++i) {
        double inner = 0;
        for (int j=0; j<pred->cols; ++j) {
            inner += ta_exp(pred->vals[i][j]);
        }
        loss += ta_ln(inner) - pred->vals[i][(int) label->vals[0][i]];
    }

    return loss / pred->rows;
}

// takes in a vector to and compute softmax on
// that vector
void softmax_single(double* arr, int size) {
    assert(0 <= size <= sizeof(arr) / sizeof(double));

	double m, sum, constant;

	m = -inf;
	for (int i = 0; i < size; ++i) {
		if (m < arr[i]) {
			m = arr[i];
		}
	}

	sum = 0.0;
	for (int i = 0; i < size; ++i) {
		sum += exp(arr[i] - m);
	}

	constant = m + log(sum);
	for (int i = 0; i < size; ++i) {
		arr[i] = exp(arr[i] - constant);
	}
}

// takes in matrix and computes soft-max along
// rows of that matrix
void softmax(matrix_t* m) {
    assert(0 <= m->rows && 0 <= m->cols);
    
    for (int i=0; i < m->rows; ++i) {
        softmax_single(m->vals[i], m->cols);
    }
}

// both matries are row-major where 
// each row is prediction/truth for a 
// single input
double mean_cross_entropy_softmax(matrix_t* logits, matrix_t* labels) {
    assert(logits->rows == labels->rows && logits->cols == labels->cols);
    
    matrix_t* pred = copy_matrix(logits);
    softmax(pred);
    apply_matrix(ta_ln, pred);

    double sum = 0;
    for (int i=0; i < pred->rows; ++i) {
        for (int j=0; j < pred->cols; ++j) {
            sum += labels->vals[i][j] * pred->vals[i][j];
        }
    }

    destroy_matrix(pred);
    return -sum;
}

// both matries are row-major where 
// each row is prediction/truth for a 
// single input, derivative of cross-
// entropy softmax
matrix_t* d_mean_cross_entropy_softmax(matrix_t* logits, matrix_t* labels) {
    assert(logits->rows == labels->rows && logits->cols == labels->cols);
    
    matrix_t* pred = copy_matrix(logits);

    subtract_matrix(pred, labels, pred);

    return pred;
}
