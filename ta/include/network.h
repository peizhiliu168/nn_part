#ifndef TA_NETWORK_H
#define TA_NETWORK_H

#include "matrix.h"

typedef matrix_t (*activation_function)(matrix_t);
typedef matrix_t (*activation_function_d)(matrix_t);
typedef float (*loss_function)(matrix_t, matrix_t);
typedef float (*loss_function_d)(matrix_t, matrix_t);

typedef struct layer {
    int prev_neurons;
    int curr_neurons;

    matrix_t weights;
    matrix_t bias;

    int loaded_start;
    int loaded_end;

} layer_t;


typedef struct network {
    int n_layers;
    int* n_neurons;

    layer_t* layers;

    int loaded_start;
    int loaded_end;

    // hyperparameters
    float learning_rate;
    int batch_size;

    // methods
    activation_function Activation;
    activation_function_d Activation_d;
    loss_function Loss;
    loss_function_d Loss_d;

} network_t;


#endif 
