#ifndef TA_NETWORK_H
#define TA_NETWORK_H

#include "matrix.h"

typedef double (*activation_function)(matrix_t);
typedef double (*activation_function_d)(matrix_t);
typedef double (*loss_function)(matrix_t, matrix_t);
typedef double (*loss_function_d)(matrix_t, matrix_t);

typedef struct layer {
    int prev_neurons;
    int curr_neurons;

    matrix_t* weights;
    matrix_t* d_weights;
    matrix_t* bias;
    matrix_t* d_bias;

    // partial loading sub-layer within layer
    int loaded_start;
    int loaded_end;

    // needed for backprop
    matrix_t* inputs;
    matrix_t* error;

} layer_t;


typedef struct network {
    int n_layers;
    layer_t** layers;

    // partial loading layer within network
    int loaded_start;
    int loaded_end;

    // hyperparameters
    double learning_rate;
    int batch_size;

    // methods
    activation_function Activation;
    activation_function_d Activation_d;
    loss_function Loss;
    loss_function_d Loss_d;

} network_t;

int init_network(void);

void destroy_network(void);

#endif 
