#include <loss.h>
#include <math_TA.h>

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

matrix_t* softmax(matrix_t* m) {
    
}

double 