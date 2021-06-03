#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <storage.h>
#include <network.h>

// #define GET_LAYER_FIELD(layer, i) ()

void* layer_to_buffer(layer_t* layer) {
    int w_size = sizeof(double) * layer->weights->rows * layer->weights->cols;
    int dw_size = sizeof(double) * layer->d_weights->rows * layer->d_weights->cols;
    int b_size = sizeof(double) * layer->bias->rows * layer->bias->cols; 
    int db_size = sizeof(double) * layer->d_bias->rows * layer->d_bias->cols; 
    int input_size = sizeof(double) * layer->inputs->rows * layer->inputs->cols; 
    int output_size = sizeof(double) * layer->outputs->rows * layer->outputs->cols; 
    
    int total_size = 11*sizeof(int); // Number of metadata

    total_size += 2*sizeof(int); // prev_neuron, curr_neuron
    total_size += 2*sizeof(int); // loaded_start and loaded_end
    total_size += 1*sizeof(int); //
    total_size += w_size; // weights
    total_size +=  dw_size; // d_weights
    total_size += b_size; // bias
    total_size += db_size; // d_bias
    total_size += input_size; // inputs
    total_size += output_size; // outputs

    char* buffer = TEE_Malloc(total_size, TEE_MALLOC_FILL_ZERO);

    int all_sizes[11] = {sizeof(int), sizeof(int), sizeof(int), sizeof(int), sizeof(int), 
                        w_size, dw_size, b_size, db_size, input_size, output_size};
    matrix_t* matrix_vals[11] = {NULL, NULL, NULL, NULL, NULL, 
                                layer->weights, layer->d_weights, 
                                layer->bias, layer->d_bias, 
                                layer->inputs, layer->outputs};
    int int_vals[11] = {layer->prev_neurons, layer->curr_neurons, layer->loaded_start,
                        layer->loaded_end, layer->inputs->rows, NULL, NULL,
                        NULL, NULL, NULL, NULL};

    // [1, prev_neur, 1, curr_neurons, 1, loaded_start, 1, loaded_end]
    int curr_index = 0;
    for (int i=0; i < 11; ++i) {
        int* tmp0 = buffer[curr_index];
        tmp0[0] = all_sizes[i];
        curr_index += sizeof(int);

        if (matrix_vals[i] != NULL) { // we are a matrix
            matrix_t* m = matrix_vals[i];
            double* tmp = buffer[curr_index];
            
            for (int r=0; r < m->rows; ++r) {
                for (int c=0; c < m->cols; ++c) {
                    tmp[r * m->cols + c] = m->vals[r][c];
                }
            }

            curr_index += all_sizes[i];

        } else if (int_vals[i] != NULL) { // we are an int
            int* tmp2 = buffer[curr_index];
            tmp2[0] = int_vals[i];
            curr_index += all_sizes[i];
        } else {
            DMSG("something wrong... or uninitialized");
        }

    }

    return (void*) buffer;
}