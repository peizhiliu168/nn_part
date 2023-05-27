#include <activations.h>
#include <math_TA.h>

float sigmoid(float x) {
    return 1 / (1 + ta_exp(-x));
}

float d_sigmoid(float x) {
    return sigmoid(x) * (1 - sigmoid(x));
}

float relu(float x) {
    return MAX(0.0, x);
}

float d_relu(float x) {
    if (x < 0) {
        return 0.0;
    }
    return 1.0;
}