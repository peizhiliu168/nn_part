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
        DMSG("destorying layer %d\n", i);
        if (!nn->layers[i]) {
            break;
        }
        destroy_matrix(nn->layers[i]->weights);
        DMSG("section %d", 1);
        destroy_matrix(nn->layers[i]->d_weights);
        DMSG("section %d", 2);
        destroy_matrix(nn->layers[i]->bias);
        DMSG("section %d", 3);
        destroy_matrix(nn->layers[i]->d_bias);
        TEE_InitSctrace();
        TEE_AddSctrace(1);
        TEE_GetSctrace(1);
        DMSG("section %d", 4);
        for (int j=0; j < 10; ++j){
            for (int k=0; k<10; ++k) {
                DMSG("%d \n", nn->layers[i]->inputs->vals[j][k]);
            }
            DMSG("\n");
        }
        
        destroy_matrix(nn->layers[i]->inputs);
        DMSG("section %d", 5);
        destroy_matrix(nn->layers[i]->outputs);
        DMSG("destoryed layer %d matrices\n", i);
        TEE_Free(nn->layers[i]);
    }
    DMSG("destorying layers struct\n");
    TEE_Free(nn->layers);
L1:
    DMSG("destorying network\n");
    TEE_Free(nn);
    DMSG("network destoryed!\n");
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
        DMSG("rows: %d, cols: %d\n", inputs->rows, inputs->cols);
        destroy_matrix(layer->inputs);
        destroy_matrix(layer->outputs);
        layer->inputs = inputs;

        matrix_t* outputs = create_matrix(inputs->rows, layer->curr_neurons);
        mult_matrix(inputs, layer->weights, outputs);
        add_matrix_element(outputs, layer->bias, outputs);
        apply_matrix(nn->Activation, outputs);

        // note: the output pointer is shared by the next layer
        // as the next layer's input
        layer->outputs = outputs;
        inputs = outputs;

        DMSG("propagated through layer %d\n", i);
    }

    DMSG("completed forward propagaion\n");
    return nn->Loss(nn->layers[nn->n_layers - 1]->outputs, labels);
}

// labels are in row-major order
void backward(network_t* nn, matrix_t* labels) {
    DMSG("starting back propagation\n");
    assert(nn != NULL && labels != NULL);

    // may need to load some sublayers here...

    matrix_t* d_outputs = nn->Loss_d(nn->layers[nn->n_layers - 1]->outputs, labels);
    for (int i=(nn->n_layers - 1); i >= 0; --i) {
        // may need to load some layers here...

        DMSG("backprop in layer %d\n", i);

        layer_t* layer = nn->layers[i];

        // d_scores = d_outputs * self.d_activate(self.a)
        matrix_t* d_scores = copy_matrix(layer->outputs);
        apply_matrix(nn->Activation_d, d_scores);
        mult_matrix_element(d_outputs, d_scores, d_scores);

        // # self.d_b:
        // #     Derivatives of the loss w.r.t the bias, averaged over all data points.
        // #     A matrix of shape (1, H)
        // #     H is the number of hidden units in this layer.
        // self.d_b = np.sum(d_scores, axis=0, keepdims=True)
        destroy_matrix(layer->d_bias);
        layer->d_bias = col_sum_matrix(d_scores);
        
        // # self.d_w:
        // #     Derivatives of the loss w.r.t the weight matrix, averaged over all data points.
        // #     A matrix of shape (H_-1, H)
        // #     H_-1 is the number of hidden units in previous layer
        // #     H is the number of hidden units in this layer.
        // self.d_w = np.dot(self.inputs.T, d_scores)
        // self.inputs = layer->inputs
        destroy_matrix(layer->d_weights);
        matrix_t* inputs_T = transpose_matrix(layer->inputs);
        layer->d_weights = create_matrix(layer->weights->rows, layer->weights->cols);
        mult_matrix(inputs_T, d_scores, layer->d_weights);
        destroy_matrix(inputs_T);


        //    def backprop(self, labels):
        // """Backward propagate the gradients/derivatives through the network.
        // Iteratively propagate the gradients/derivatives (starting from
        // outputs) through each layer, and save gradients/derivatives of
        // each parameter (weights or bias) in the layer.
        // """
        // d_outputs = self.d_loss(self.layers[-1].a, labels)
        // for layer in self.layers[::-1]:
        //     d_inputs = layer.backward(d_outputs)
        //     d_outputs = d_inputs

        

        // d_inputs = np.dot(d_scores, self.w.T)
        matrix_t* W_T = transpose_matrix(layer->weights);
        matrix_t* d_inputs = create_matrix(d_scores->rows, W_T->cols);
        mult_matrix(d_scores, W_T, d_inputs);
        destroy_matrix(W_T);

        // self.d_b /= d_scores.shape[0]
        // self.d_w /= d_scores.shape[0]
        matrix_t* n = create_matrix(1,1);
        n->vals[0][0] = d_scores->rows;
        div_matrix_element(layer->d_bias, n, layer->d_bias);
        div_matrix_element(layer->d_weights, n, layer->d_weights);
        destroy_matrix(n);

        destroy_matrix(d_scores);
        destroy_matrix(d_outputs);
        d_outputs = d_inputs;

        DMSG("input rows: %d, cols: %d\n", layer->inputs->rows, layer->inputs->cols);
        DMSG("output rows: %d, cols: %d\n", layer->outputs->rows, layer->outputs->cols);
        DMSG("d_weights rows: %d, cols: %d\n", layer->d_weights->rows, layer->d_weights->cols);
        DMSG("d_bias rows: %d, cols: %d\n", layer->d_bias->rows, layer->d_bias->cols);
    }
    destroy_matrix(d_outputs);

    DMSG("finished backprop!\n");

    return;
}
