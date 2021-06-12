#include <activations.h>
#include <math_TA.h>

double sigmoid(double x) {
    return 1 / (1 + ta_exp(-x));
}

double d_sigmoid(double x) {
    return sigmoid(x) * (1 - sigmoid(x));
}

double relu(double x) {
    return MAX(0.0, x);
}

double d_relu(double x) {
    if (x < 0) {
        return 0.0;
    }
    return 1.0;
}