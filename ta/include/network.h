#ifndef TA_NETWORK_H
#define TA_NETWORK_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include "matrix.h"
#include "optimizer.h"


typedef float (*activation_function)(float);
typedef float (*activation_function_d)(float);
typedef float (*loss_function)(matrix_t*, matrix_t*);
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
    size_t* layer_offsets;

    // partial loading layer within network
    int n_loaded; // maximum number of layers loaded in network
    int loaded_start;
    int loaded_end;

    // hyperparameters
    int batch_size;
    float learning_rate;
    optimizer_type_t optimizer;

    // methods
    activation_function Activation;
    activation_function_d Activation_d;
    loss_function Loss;
    loss_function_d Loss_d;

    // transiet TEE key
    uint32_t key_size;
    TEE_ObjectHandle aeskey;

    // shared memory buffer pointer
    void* shmem;
    uint32_t shmem_size;

} network_t;

network_t* allocate_network(void);

void init_network(void);

void destroy_network(void);

layer_t* create_layer(int prev_neurons, int curr_neurons, bool store, int layer_number, size_t* offset);

void destroy_layer(layer_t* layer);

float forward(matrix_t* features, matrix_t* labels);

void backward(matrix_t* labels);

void train(int epochs);

matrix_t* predict(matrix_t* features);

float accuracy(matrix_t* y_hat, matrix_t* labels);

void swap_layers(int start, int end, bool train);

extern network_t* nn;

#endif 
