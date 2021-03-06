#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <network.h>
#include <loss.h>
#include <activations.h>
#include <assert.h>
#include <optimizer.h>
#include <data.h>
#include <math_TA.h>

network_t* nn;

void init_network(void) {
    //DMSG("initializing network\n");
    // parameters we should pass in later
    int n_layers = 3;
    int layers[] = {784, 64, 32, 10};
    int loaded_start = 0;
    int n_loaded = 3;
    double learning_rate = 0.03;
    int batch_size = 50;
    optimizer_type_t optimizer = GD;

    // initialize original network
    nn = calloc(1, sizeof(network_t));
    if (!nn) {
        assert(false);
    }

    // set network parameters
    nn->n_layers = n_layers;
    nn->n_loaded = n_loaded;
    nn->loaded_start = loaded_start;
    nn->loaded_end = loaded_start + n_loaded;
    nn->learning_rate = learning_rate;
    nn->batch_size = batch_size;
    nn->optimizer = optimizer;
    //DMSG("set network parameters\n");

    assert(nn->n_layers >= nn->n_loaded);

    // initialize network layers
    nn->layers = calloc(nn->n_loaded, sizeof(layer_t*));
    if (!nn->layers) {
        destroy_network();
        assert(false);    
    }
    for (int i=0; i<n_layers; ++i) {
        //DMSG("creating layer: %d\n", i);
        if (i < nn->n_loaded) {
            // create layers that exist in the struct
            nn->layers[i] = create_layer(layers[i], layers[i+1], 0, i);
        } else {
            // create layers that exist in storage
            create_layer(layers[i], layers[i+1], 1, i);
        }       
    }

    // set activation and loss functions
    nn->Activation = relu;
    nn->Activation_d = d_relu;
    nn->Loss = mean_cross_entropy_softmax;
    nn->Loss_d = d_mean_cross_entropy_softmax;

    //DMSG("finished initializing network\n");
    //return nn;
}

void destroy_network(void) {
    //DMSG("destroying network...\n");
    if (!nn) {
        return;
    }

    if (!nn->layers) {
        goto L1;
    }

    for (int i=0; i < (nn->loaded_end - nn->loaded_end); ++i) {
        //DMSG("destorying layer %d\n", i);
        layer_t* layer = nn->layers[i];
        if (!layer) {
            continue;
        }
        destroy_layer(layer);
    }
    //DMSG("destorying layers struct\n");
    free(nn->layers);
L1:
    //DMSG("destorying network\n");
    free(nn);
    //DMSG("network destoryed!\n");
}

layer_t* create_layer(int prev_neurons, int curr_neurons, int store, int layer_number) {
    layer_t* layer = calloc(1, sizeof(layer_t));
    assert(layer != NULL);

    layer->prev_neurons = prev_neurons;
    layer->curr_neurons = curr_neurons;

    layer->weights = create_matrix_random(layer->prev_neurons, layer->curr_neurons, 
                                    -ta_sqrt(2.0 / layer->prev_neurons), ta_sqrt(2.0 / layer->prev_neurons));
    layer->d_weights = create_matrix(1,1); // placeholder
    layer->bias = create_matrix(1, layer->curr_neurons);
    layer->d_bias = create_matrix(1, 1); // placeholder
    
    layer->loaded_start = 0;
    layer->loaded_end = layer->curr_neurons;

    layer->inputs = create_matrix(1,1); // placeholder
    layer->outputs = create_matrix(1,1); // placeholder

    if (store) {
        //store_layer(layer, layer_number);
        assert(0);
        return NULL;
    }
    return layer;
}

void destroy_layer(layer_t* layer) {
    if (!layer){
        return;
    }
    if (layer->weights) {
        //DMSG("section 1\n");
        destroy_matrix(layer->weights);
    }
    if (layer->d_weights) {
        //DMSG("section 2\n");
        destroy_matrix(layer->d_weights);
    }
    if (layer->bias) {
        // DMSG("section 3\n");
        destroy_matrix(layer->bias);
    }
    if (layer->d_bias) {
        // DMSG("section 3\n");
        destroy_matrix(layer->d_bias);
    }
    if (layer->inputs) {
        // DMSG("section 4\n");
        destroy_matrix(layer->inputs);
    }
    if (layer->outputs) {
        // DMSG("section 5\n");
        destroy_matrix(layer->outputs);
    }
    // DMSG("section 6\n");
    free(layer);
}

// input and labels are in row-major order, meaning 
// each row corresponds to a particular training example. 
double forward(matrix_t* features, matrix_t* labels) {
    //DMSG("starting forward propagation\n");
    assert(nn != NULL && features != NULL);

    int training = labels != NULL;

    matrix_t* outputs = copy_matrix(features);

    int i;
    for (int l=0; l < ((nn->n_layers - 1) / nn->n_loaded + 1); ++l) {
        int start = l * nn->n_loaded;
        int end = MIN((l + 1) * nn->n_loaded, nn->n_layers);
        
        // DMSG("swapping layers %d to %d\n", start, end);
        swap_layers(start, end, training);
        
        // DMSG("forward layers %d to %d\n", start, end);
        for (i=0; i < (end - start); ++i) {
            // matrix_t* inputs = copy_matrix(outputs);
            matrix_t* inputs = outputs;

            layer_t* layer = nn->layers[i];
            //DMSG("rows: %d, cols: %d\n", inputs->rows, inputs->cols);
            // DMSG("destroyed previous input and output\n");
            destroy_matrix(layer->inputs);
            destroy_matrix(layer->outputs);
            layer->inputs = inputs;

            // DMSG("starting forward computation\n");
            outputs = create_matrix(inputs->rows, layer->curr_neurons);
            mult_matrix(inputs, layer->weights, outputs);
            add_matrix_element(outputs, layer->bias, outputs);
            apply_matrix(nn->Activation, outputs);
            // DMSG("finished forward computation\n");

            // note: the output pointer is not shared by the next layer
            // as the next layer's input, this makes things a lot 
            // easier to manage
            layer->outputs = outputs;

            //DMSG("propagated through layer %d\n", i);
            outputs = copy_matrix(outputs);
        }
    }
    destroy_matrix(outputs);

    // DMSG("completed forward propagaion\n");
    if (training) {
        
        return nn->Loss(nn->layers[(nn->n_layers - 1) % nn->n_loaded]->outputs, labels);
    }
    
    return -1.0;
}

// labels are in row-major order
void backward(matrix_t* labels) {
    // DMSG("starting back propagation\n");
    
    assert(nn != NULL && labels != NULL);

    // may need to load some layers here...
    matrix_t* d_outputs = nn->Loss_d(nn->layers[(nn->n_layers - 1) % nn->n_loaded]->outputs, labels);
    for (int l=0; l < ((nn->n_layers - 1) / nn->n_loaded + 1); ++l) {
        int start = nn->n_layers - MIN((l + 1) * nn->n_loaded, nn->n_layers);
        int end = nn->n_layers - l * nn->n_loaded;
        
        // DMSG("back swapping layers %d to %d\n", start, end);
        swap_layers(start, end, 1);
        
        // DMSG("back layers %d to %d\n", start, end);
        
        for (int i=((end - start) - 1); i >= 0; --i) {
            
            // may need to load some layers here...

            //DMSG("backprop in layer %d\n", i);

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

            // //DMSG("input rows: %d, cols: %d\n", layer->inputs->rows, layer->inputs->cols);
            // //DMSG("output rows: %d, cols: %d\n", layer->outputs->rows, layer->outputs->cols);
            // //DMSG("d_weights rows: %d, cols: %d\n", layer->d_weights->rows, layer->d_weights->cols);
            // //DMSG("d_bias rows: %d, cols: %d\n", layer->d_bias->rows, layer->inputs->cols);
            
        }
    }
    destroy_matrix(d_outputs);
    

    //DMSG("finished backprop!\n");
    
    return;
}

// trains the network with the optimizer
void train(int epochs) {
    assert(nn != NULL && data_loader != NULL);
    

    // start large training loop
    for (int epoch=0; epoch < epochs; ++epoch) {
        
        
        double sum_loss = 0.0;
        int batch_size = nn->batch_size;
        int b = 0;
        for (b=0; b < ((data_loader->N - 1) / batch_size + 1); ++b) { // iterate through batches
            
            matrix_t* batch_features = copy_submatrix(data_loader->features, 
                                                        b * batch_size, MIN((b + 1) * batch_size, 
                                                                            data_loader->features->rows),
                                                        0, data_loader->features->cols);
            matrix_t* batch_labels = copy_submatrix(data_loader->labels, 
                                                    b * batch_size, MIN((b + 1) * batch_size,
                                                                        data_loader->labels->rows),
                                                    0, data_loader->labels->cols);

            double loss = forward(batch_features, batch_labels);
            // if (isnan(loss) || isinf(loss)) {
            //     //DMSG("abnormal loss in epoch %d, batch %d\n", epoch, b);
            //     continue;
            // }
            
            sum_loss += loss;

            backward(batch_labels);
            update();

            destroy_matrix(batch_features);
            destroy_matrix(batch_labels);

            
            printf("batch %d loss: %f\n", b, loss);
        }
        sum_loss /= (b + 1);
        matrix_t* y_hat = predict(data_loader->features);
        double acc = accuracy(y_hat, data_loader->labels);
        destroy_matrix(y_hat);
        
        printf("========= Epoch: %d, Loss: %f, Accuracy: %f ==========", epoch, sum_loss, acc);
    }
    
}

// given a matrix of features in row-major order, 
// use the model for inference
matrix_t* predict(matrix_t* features) {
    // for (int i=0; i < features->rows; ++i) {

    //     forward()
    // }
    forward(features, NULL);
    matrix_t* y_hat = copy_matrix(nn->layers[(nn->n_layers - 1) % nn->n_loaded]->outputs);
    softmax(y_hat); // changes y_hat
    return y_hat;
}

// given a matrix of features in row-major order, 
// use the model for inference
// matrix_t* predict(matrix_t* features) {
//     // DMSG("creating matrix\n");
//     matrix_t* y_hat = create_matrix(features->rows, data_loader->classes);

//     for (int i=0; i < features->rows; ++i) {
//         // DMSG("iterated through feature %d\n", i);
//         matrix_t* feature = copy_submatrix(features, i, i+1, 0, features->cols);
//         forward(feature, NULL);
//         for (int j=0; j < data_loader->classes; ++j) {
//             y_hat->vals[i][j] = nn->layers[(nn->n_layers - 1) % nn->n_loaded]->outputs->vals[0][j];
//         }
//         destroy_matrix(feature);
//     }
//     // forward(features, NULL);
//     // matrix_t* y_hat = copy_matrix(nn->layers[(nn->n_layers - 1) % nn->n_loaded]->outputs);
//     softmax(y_hat); // changes y_hat
//     return y_hat;
// }


// given predictions and labels, calculate accuracy
double accuracy(matrix_t* y_hat, matrix_t* labels) {
    assert(y_hat->rows == labels->rows && y_hat->cols == labels->cols);

    double correct = 0;
    for (int i=0; i < y_hat->rows; ++i) {
        int max_index = 0;
        double max_value = 0.0;
        for (int j=0; j < y_hat->cols; ++j) {
            if (y_hat->vals[i][j] > max_value) {
                max_value = y_hat->vals[i][j];
                max_index = j;
            }
        }
        correct += labels->vals[i][max_index];
    }

    return correct / ((double) (y_hat->rows));
}

// sawp the current layers in nn with the new layers
// start layer is inclusive and end layer is exclusive
void swap_layers(int start, int end, int train) {
    assert(nn != NULL);

    if (start == nn->loaded_start && end == nn->loaded_end) {
        return;
    }

    // assert(start >= 0 && start < nn->n_layers);
    // assert(end > start && end <= nn->n_layers);
    // assert((end - start) <= nn->n_loaded);

    // for (int i=0; i < nn->n_loaded; ++i) {
    //     int store_index = nn->loaded_start + i;
    //     if (store_index < nn->loaded_end) {
    //         if (train) { // only store layers when we are in training mode
    //             store_layer(nn->layers[i], store_index);
    //         } else { // if we're in predicting mode, just destroy layer without storing
    //             destroy_layer(nn->layers[i]);
    //         }
    //     }
        
    //     int load_index = start + i;
    //     if (load_index < end) {
    //         nn->layers[i] = read_layer(load_index);
    //     }
    // }
    
    // nn->loaded_start = start;
    // nn->loaded_end = end;
    assert(0);
}
