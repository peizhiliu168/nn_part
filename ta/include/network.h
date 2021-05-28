#ifndef TA_NETWORK_H
#define TA_NETWORK_H

#include "matrix.h"

typedef double (*activation_function)(double);
typedef double (*activation_function_d)(double);
typedef double (*loss_function)(matrix_t*, matrix_t*);
typedef matrix_t* (*loss_function_d)(matrix_t*, matrix_t*);

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

    // forward output
    matrix_t* outputs;

} network_t;

network_t* init_network(void);

void destroy_network(network_t* nn);

double forward(network_t* nn, matrix_t* features, matrix_t* labels);

void backward(network_t* nn, matrix_t* labels);

#endif 
