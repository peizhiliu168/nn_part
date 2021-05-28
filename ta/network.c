#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <network.h>
#include <loss.h>
#include <activations.h>
#include <assert.h>

network_t* init_network(void) {
    DMSG("initializing network\n");
    // parameters we should pass in later
    int n_layers = 5;
    int layers[] = {784, 10, 10, 10, 10, 10};
    int loaded_start = 0;
    int loaded_end = 4;
    double learning_rate = 1e-3;
    int batch_size = 200;

    // initialize original network
    network_t* nn = TEE_Malloc(sizeof(network_t), TEE_MALLOC_FILL_ZERO);
    if (!nn) {
        assert(false);
    }

    // set network parameters
    nn->n_layers = n_layers;
    nn->loaded_start = loaded_start;
    nn->loaded_end = loaded_end;
    nn->learning_rate = learning_rate;
    nn->batch_size = batch_size;
    DMSG("set network parameters\n");

    // initialize network layers
    nn->layers = TEE_Malloc(sizeof(layer_t*) * nn->n_layers, TEE_MALLOC_FILL_ZERO);
    if (!nn->layers) {
        destroy_network(nn);
        assert(false);    
    }
    for (int i=0; i<n_layers; ++i) {
        layer_t* layer = TEE_Malloc(sizeof(layer_t), TEE_MALLOC_FILL_ZERO);
        if (!layer) {
            destroy_network(nn);
            assert(false);
        }

        layer->prev_neurons = layers[i];
        layer->curr_neurons = layers[i+1];

        // subject to change...
        layer->loaded_start = 0;
        layer->loaded_end = layer->curr_neurons;

        layer->weights = create_matrix_random(layer->prev_neurons, layer->curr_neurons, -1, 1);
        layer->bias = create_matrix_random(1, layer->curr_neurons, -1, 1);
        // layer->inputs = create_matrix(layer->prev_neurons, 1);
        // layer->error = create_matrix(layer->curr_neurons, 1);

        nn->layers[i] = layer;
        DMSG("created layer %d with %d and %d neurons\n", i, layer->prev_neurons, layer->curr_neurons);
    }

    // set activation and loss functions
    nn->Activation = relu;
    nn->Activation_d = d_relu;
    nn->Loss = mean_cross_entropy_softmax;
    nn->Loss_d = d_mean_cross_entropy_softmax;

    DMSG("finished initializing network\n");
    return nn;
}

void destroy_network(network_t* nn) {
    DMSG("destroying network...\n");
    if (!nn) {
        return;
    }

    if (!nn->layers) {
        goto L1;
    }

    for (int i=0; i<nn->n_layers; ++i) {
        if (!nn->layers[i]) {
            break;
        }
        destroy_matrix(nn->layers[i]->weights);
        destroy_matrix(nn->layers[i]->d_weights);
        destroy_matrix(nn->layers[i]->bias);
        destroy_matrix(nn->layers[i]->d_bias);
        destroy_matrix(nn->layers[i]->inputs);
        destroy_matrix(nn->layers[i]->error);
        TEE_Free(nn->layers[i]);
    }

    TEE_Free(nn->layers);
L1:
    TEE_Free(nn);
}


// input and labels are in row-major order, meaning 
// each row corresponds to a particular training example. 
double forward(network_t* nn, matrix_t* features, matrix_t* labels) {
    DMSG("starting forward propagation\n");
    assert(nn != NULL && features != NULL && labels != NULL);

    matrix_t* inputs = copy_matrix(features);

    for (int i=0; i < nn->n_layers; ++i) {
        // conditional load layers here...

        layer_t* layer = nn->layers[i];

        destroy_matrix(layer->inputs);
        layer->inputs = inputs;

        matrix_t* outputs = create_matrix(inputs->rows, layer->curr_neurons);
        mult_matrix(inputs, layer->weights, outputs);
        add_matrix(outputs, layer->bias, outputs);
        apply_matrix(nn->Activation, outputs);

        inputs = outputs;

        DMSG("propagated through layer %d\n", i);
    }

    nn->outputs = inputs;

    DMSG("completed forward propagaion\n");
    return nn->Loss(nn->outputs, labels);
}

// lables are in row-major order
void backward(network_t* nn, matrix_t* labels) {
    return;
}
