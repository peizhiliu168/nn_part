#ifndef TA_NETWORK_H
#define TA_NETWORK_H

#include "matrix.h"
#include "optimizer.h"


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
    matrix_t* outputs;

} layer_t;


typedef struct network {
    int n_layers;
    layer_t** layers;

    // partial loading layer within network
    int n_loaded; // maximum number of layers loaded in network
    int loaded_start;
    int loaded_end;

    // hyperparameters
    int batch_size;
    double learning_rate;
    optimizer_type_t optimizer;

    // methods
    activation_function Activation;
    activation_function_d Activation_d;
    loss_function Loss;
    loss_function_d Loss_d;

} network_t;

void init_network(void);

void destroy_network(void);

layer_t* create_layer(int prev_neurons, int curr_neurons, int store, int layer_number);

void destroy_layer(layer_t* layer);

double forward(matrix_t* features, matrix_t* labels);

void backward(matrix_t* labels);

void train(int epochs);

matrix_t* predict(matrix_t* features);

double accuracy(matrix_t* y_hat, matrix_t* labels);

void swap_layers(int start, int end, int train);

extern network_t* nn;

#endif 
