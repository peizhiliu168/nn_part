#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <assert.h>

#include <storage.h>
#include <network.h>

void* layer_to_buffer(layer_t* layer, size_t* out_size) {
    size_t int_type_size = 4 * sizeof(serialize_t);
    size_t int_data_size = 4 * sizeof(int);
    size_t matrix_type_size = 6 * sizeof(serialize_t);
    size_t matrix_data_size = sizeof(double) * (
                                                layer->weights->rows * layer->weights->cols + 
                                                layer->d_weights->rows * layer->d_weights->cols +
                                                layer->bias->rows * layer->bias->cols + 
                                                layer->d_bias->rows * layer->d_bias->cols +
                                                layer->inputs->rows * layer->inputs->cols + 
                                                layer->outputs->rows * layer->outputs->cols
                                                );
    size_t total_size = int_type_size + int_data_size + matrix_type_size + matrix_data_size;

    uint8_t* buffer = TEE_Malloc(total_size, TEE_MALLOC_FILL_ZERO);
    assert(buffer != NULL);

    size_t used_size;
    size_t remaining_size = total_size;
    void* start = buffer;
    
    // order here is CRUCIAL
    // layer->prev_neurons
    serialize_int(layer->prev_neurons, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->curr_neurons
    serialize_int(layer->curr_neurons, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->weights
    serialize_matrix(layer->weights, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->d_weights
    serialize_matrix(layer->d_weights, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->bias
    serialize_matrix(layer->bias, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->d_bias
    serialize_matrix(layer->d_bias, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->loaded_start
    serialize_int(layer->loaded_start, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->loaded_end
    serialize_int(layer->loaded_end, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->inputs
    serialize_matrix(layer->inputs, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    // layer->outputs
    serialize_matrix(layer->outputs, buffer, remaining_size, &used_size);
    buffer += used_size;
    remaining_size -= used_size;

    *out_size = total_size;
    return start;
}

layer_t* buffer_to_layer(void* buffer, size_t size){
    layer_t* layer = TEE_Malloc(sizeof(layer_t), TEE_MALLOC_FILL_ZERO);
    assert(layer != NULL);

    size_t used_size;
    size_t remaining_size = size;
    uint8_t* byte_buffer = (uint8_t*) buffer;

    // order here is CRUCIAL
    // layer->prev_neurons
    layer->prev_neurons = deserialize_int(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->curr_neurons
    layer->curr_neurons = deserialize_int(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->weights
    layer->weights = deserialize_matrix(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->d_weights
    layer->d_weights = deserialize_matrix(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->bias
    layer->bias = deserialize_matrix(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->d_bias
    layer->d_bias = deserialize_matrix(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->loaded_start
    layer->loaded_start = deserialize_int(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->loaded_end
    layer->loaded_end = deserialize_int(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->inputs
    layer->inputs = deserialize_matrix(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    // layer->outputs
    layer->outputs = deserialize_matrix(byte_buffer, remaining_size, &used_size);
    byte_buffer += used_size;
    remaining_size -= used_size;

    return layer;
}

// |<type>|<data>|
void serialize_int(int data, void* buffer, size_t size, size_t* used) {
    size_t type_size = sizeof(serialize_t);
    size_t data_size = sizeof(int);
    size_t total_size = type_size + data_size;

    assert(size >= total_size);
    uint8_t* byte_buffer = (uint8_t*) buffer;

    *((serialize_t*) byte_buffer) = int_s;
    byte_buffer += type_size;
    *((int*) byte_buffer) = data;
    *used = total_size;
}

int deserialize_int(void* buffer, size_t size, size_t* used) {
    size_t type_size = sizeof(serialize_t);
    size_t data_size = sizeof(int);
    size_t total_size = type_size + data_size;
    
    assert(size >= total_size);
    uint8_t* byte_buffer = (uint8_t*) buffer;

    serialize_t type = *((serialize_t*) byte_buffer);
    byte_buffer += type_size;
    
    assert(type == int_s);

    *used = total_size;
    return *((int*) byte_buffer);
}

// |<type>|<meta (rows,cols)>|<data>|
void serialize_matrix(matrix_t* m, void* buffer, size_t size, size_t* used) {
    size_t type_size = sizeof(serialize_t);
    size_t row_size = sizeof(int), col_size = sizeof(int);
    size_t data_size = m->rows * m->cols * sizeof(double);
    size_t total_size = type_size + row_size + col_size + data_size;

    assert(size >= total_size);
    uint8_t* byte_buffer = (uint8_t*) buffer;

    *((serialize_t*) byte_buffer) = matrix_s;
    byte_buffer += type_size;
    *((int*) byte_buffer) = m->rows;
    byte_buffer += row_size;
    *((int*) byte_buffer) = m->cols;
    byte_buffer += col_size;
    
    for (int i=0; i < m->rows; ++i) {
        for (int j=0; j < m->cols; ++j) {
            *((double*) byte_buffer) = m->vals[i][j];
            byte_buffer += sizeof(double);
        }
    }

    *used = total_size;
}

matrix_t* deserialize_matrix(void* buffer, size_t size, size_t* used) {
    size_t type_size = sizeof(serialize_t);
    size_t row_size, col_size = sizeof(int);
    size_t inter_size = type_size + row_size + col_size;

    uint8_t* byte_buffer = (uint8_t*) buffer;

    serialize_t type = *((serialize_t*) byte_buffer);
    byte_buffer += type_size;

    assert(type == matrix_s && size >= inter_size);

    int rows = *((int*) byte_buffer);
    byte_buffer += row_size;
    int cols = *((int*) byte_buffer);
    byte_buffer += col_size;

    size_t data_size = rows * cols * sizeof(double);
    size_t total_size = inter_size + data_size;

    assert(size >= total_size);

    matrix_t* m = create_matrix(rows, cols);
    for (int i=0; i < rows; ++i) {
        for (int j=0; j < cols; ++j) {
            m->vals[i][j] = *((double*) byte_buffer);
            byte_buffer += sizeof(double);
        }
    }

    *used = total_size;
    return m;

}

/*
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

        } else { // we are neither or uninitialized
            DMSG("something wrong... or uninitialized");
        }

    }

    out_buffer = buffer;
    *out_size = total_size;
    
*/