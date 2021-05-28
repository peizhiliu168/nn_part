#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <network.h>
#include <loss.h>
#include <activations.h>

network_t* nn;

int init_network(void) {
    // parameters we should pass in later
    int n_layers = 5;
    int layers[] = {784, 10, 10, 10, 10, 10};
    int loaded_start = 0;
    int loaded_end = 4;
    double learning_rate = 1e-3;
    int batch_size = 200;

    // initialize original network
    nn = TEE_Malloc(sizeof(network_t), TEE_MALLOC_FILL_ZERO);
    if (!nn) {
        return -1;
    }

    // set network parameters
    nn->n_layers = n_layers;
    nn->loaded_start = loaded_start;
    nn->loaded_end = loaded_end;
    nn->learning_rate = learning_rate;
    nn->batch_size = batch_size;

    // initialize network layers
    nn->layers = TEE_Malloc(sizeof(layer_t*) * nn->n_layers, TEE_MALLOC_FILL_ZERO);
    if (!nn->layers) {
        destroy_network();
        return -1;
    }
    for (int i=0; i<n_layers; ++i) {
        layer_t* layer = TEE_Malloc(sizeof(layer_t), TEE_MALLOC_FILL_ZERO);
        if (!layer) {
            destroy_network();
            return -1;
        }

        layer->prev_neurons = layers[i];
        layer->curr_neurons = layers[i+1];

        // subject to change...
        layer->loaded_start = 0;
        layer->loaded_end = layer->curr_neurons;

        layer->weights = create_matrix_random(layer->prev_neurons, layer->curr_neurons, -1, 1);
        layer->bias = create_matrix_random(layer->curr_neurons, 1, -1, 1);
        layer->inputs = create_matrix(layer->prev_neurons, 1);
        layer->error = create_matrix(layer->curr_neurons, 1);

        nn->layers[i] = layer;
    }

    // set activation and loss functions
    nn->Activation = relu;
    nn->Activation_d = d_relu;
    nn->Loss = mean_cross_entropy_softmax;
    nn->Loss_d = d_mean_cross_entropy_softmax;


    return 0;
}

void destroy_network(void) {
    if (!nn) {
        return;
    }

    if (!nn->layers) {
        goto L1;
    }

    for (int i=0; i<nn->layers; ++i) {
        if (!nn->layers[i]) {
            break;
        }
        destroy_matrix(nn->layers[i].weights);
        destroy_matrix(nn->layers[i].bias);
        destroy_matrix(nn->layers[i].inputs);
        destroy_matrix(nn->layers[i].error);
        TEE_Free(nn->layers[i]);
    }

    TEE_Free(nn->layers);
L1:
    TEE_Free(nn);
}