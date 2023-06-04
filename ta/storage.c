#include <string.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <assert.h>

#include <storage.h>
#include <network.h>

// creates or overwrites existing secure object and then
// stores serialized layer into storage
void store_layer(layer_t* layer, int layer_number) {
    TEE_AddSctrace(69);
    // DMSG("attempt to store layer %d \n", layer_number);
    // get layer buffer
    size_t buffer_size;
    TEE_AddSctrace(690);
    void* buffer = layer_to_buffer(layer, &buffer_size);
    TEE_AddSctrace(690);

    // DMSG("got buffer \n");

    // create and overwrite previous object
    uint32_t storageID = TEE_STORAGE_PRIVATE;
    int objectID = layer_number;
    size_t objectIDLen = sizeof(layer_number);
    uint32_t flags = TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_SHARE_WRITE | 
                    TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ |
                    TEE_DATA_FLAG_OVERWRITE;
    TEE_ObjectHandle attributes = TEE_HANDLE_NULL;
    TEE_ObjectHandle handle;

    // for (int i=0; i < buffer_size; ++i) {
    //     DMSG("%c\n", ((char*) buffer)[i]);
    // }

    // DMSG("calling TEE_CreatePersistentObject\n");
    TEE_AddSctrace(691);
    TEE_Result res = TEE_CreatePersistentObject(storageID, &objectID,
                                                objectIDLen, flags,
                                                attributes, buffer,
                                                buffer_size, &handle);
    TEE_AddSctrace(691);
    // DMSG("res: %d\n", res);
    assert(res == TEE_SUCCESS);
    // DMSG("created presistent object for layer %d\n", layer_number);
    TEE_CloseObject(handle);

    // free buffer
    TEE_Free(buffer);
    // DMSG("freed buffer\n");

    // free layer
    destroy_layer(layer);
    // DMSG("destroyed layer %d\n", layer_number);
    TEE_AddSctrace(69);
}

// reads the secure object and deserializes layer
// back into layer_t struct
layer_t* read_layer(int layer_number) {
    TEE_AddSctrace(420);
    // open object store
    uint32_t storageID = TEE_STORAGE_PRIVATE;
    int objectID = layer_number;
    size_t objectIDLen = sizeof(layer_number);
    uint32_t flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ;
    TEE_ObjectHandle handle;

    TEE_Result res = TEE_OpenPersistentObject(storageID, &objectID,
                                                objectIDLen, flags, &handle);
    // DMSG("res: %d\n", res);
    assert(res == TEE_SUCCESS);

    // get object info (mainly to get size of object)
    TEE_ObjectInfo info;
    res = TEE_GetObjectInfo1(handle, &info);
    assert(res == TEE_SUCCESS);
    size_t size = info.dataSize;

    // get layer buffer
    size_t read_size;
    void* buffer = TEE_Malloc(size, TEE_MALLOC_FILL_ZERO);
    TEE_AddSctrace(4200);
    res = TEE_ReadObjectData(handle, buffer, size, &read_size);
    TEE_AddSctrace(4200);
    // DMSG("res: %d\n", res);
    assert(res == TEE_SUCCESS);
    assert(read_size == size);

    TEE_CloseObject(handle);

    // DMSG("starting buffer to layer conversion for %d\n", layer_number);
    // convert layer buffer to layer
    TEE_AddSctrace(4201);
    layer_t* layer = buffer_to_layer(buffer, size);
    TEE_AddSctrace(4201);
    // DMSG("completed buffer to layer conversion for %d\n", layer_number);
    
    // free buffer
    TEE_Free(buffer);
    TEE_AddSctrace(420);
    return layer;
}

// sends a secure layer to the REE
void store_layer_SHM(layer_t* layer, int layer_number) {
    DMSG("Store layer, %d\n", layer_number);
    TEE_AddSctrace(69);

    // Layer serialization into buffer
    size_t buffer_size;
    TEE_AddSctrace(690);
    void* buffer = layer_to_buffer(layer, &buffer_size);
    void* enc_buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
    TEE_AddSctrace(690);

    

    // Define operations object
    TEE_Result res;
	TEE_OperationHandle op_enc;
	res = TEE_AllocateOperation(&op_enc, TEE_ALG_AES_CTR, TEE_MODE_ENCRYPT, nn->key_size);
    if (res != TEE_SUCCESS) {
        EMSG("Store - allocate operation failed\n");
    }

	res = TEE_SetOperationKey(op_enc, nn->aeskey);
    if (res != TEE_SUCCESS) {
        EMSG("Store - set operation key failed\n");
    }

    // Encrypt the buffer
    size_t IVlen = 16;
    void* IV = TEE_Malloc(IVlen, 0);
    TEE_CipherInit(op_enc, IV, IVlen);

    
    uint32_t encrypted_size = buffer_size;
    TEE_CipherUpdate(op_enc, buffer, buffer_size, enc_buffer, &encrypted_size);
    if (res != TEE_SUCCESS) {
        EMSG("Store - Cipher update failed 0x%x\n", res);
    }

    DMSG("=============================== Encrypt layer, buffer size: %d, enc size: %d\n", buffer_size, encrypted_size);

    res = TEE_CipherDoFinal(op_enc, buffer, buffer_size, enc_buffer, &encrypted_size);
    if (res != TEE_SUCCESS) {
        EMSG("Store - Cipher do final failed 0x%x\n", res);
    }

    TEE_Free(IV);


    // Free operations object

    TEE_FreeOperation(op_enc);

    // Send buffer to shared memory
    size_t offset = nn->layer_offsets[layer_number];
    DMSG("Store layer\n");
    DMSG("Storing layer in SHMEM: offset %d, content: %d\n", offset, *((uint32_t*) nn->shmem));

    uint8_t* shmem = ((uint8_t*) nn->shmem);

    *(size_t*)(shmem + offset) = buffer_size;

    // *((size_t*) shmem[offset]) = buffer_size;
    DMSG("Stored buffer size\n");
    TEE_MemMove(shmem + offset + sizeof(size_t), enc_buffer, buffer_size);
    DMSG("Stored layer\n");

    // free buffer and layer
    TEE_Free(buffer);
    TEE_Free(enc_buffer);
    destroy_layer(layer);

    TEE_AddSctrace(69);
}

// reads the secure object and deserializes layer
// back into layer_t struct
layer_t* read_layer_SHM(int layer_number) {
    DMSG("Read layer: %d\n", layer_number);
    TEE_AddSctrace(420);
    // // open object store
    // uint32_t storageID = TEE_STORAGE_PRIVATE;
    // int objectID = layer_number;
    // size_t objectIDLen = sizeof(layer_number);
    // uint32_t flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ;
    // TEE_ObjectHandle handle;

    // TEE_Result res = TEE_OpenPersistentObject(storageID, &objectID,
    //                                             objectIDLen, flags, &handle);
    // // DMSG("res: %d\n", res);
    // assert(res == TEE_SUCCESS);

    // // get object info (mainly to get size of object)
    // TEE_ObjectInfo info;
    // res = TEE_GetObjectInfo1(handle, &info);
    // assert(res == TEE_SUCCESS);
    // size_t buffer_size = info.dataSize;

    size_t offset = nn->layer_offsets[layer_number];

    uint8_t* shmem = ((uint8_t*) nn->shmem);

    size_t buffer_size = *(size_t*)(shmem + offset);
    DMSG("Reading layer from SHMEM: offset %d, size: %d\n", offset, buffer_size);


    // get layer buffer
    size_t read_size;
    void* buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
    void* enc_buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
    TEE_MemMove(enc_buffer, (void*)(shmem + offset + sizeof(size_t)), buffer_size);

    // Define operations object
    TEE_Result res;
	TEE_OperationHandle op_dec;

	res = TEE_AllocateOperation(&op_dec, TEE_ALG_AES_CTR, TEE_MODE_DECRYPT, nn->key_size);
    if (res != TEE_SUCCESS) {
        EMSG("Load - Allocation operation failed: 0x%x\n", res);
    }

	res = TEE_SetOperationKey(op_dec, nn->aeskey);
    if (res != TEE_SUCCESS) {
        EMSG("Load - Set operation key failed 0x%x\n", res);
    }

    // Decrypt the buffer
    size_t IVlen = 16;
    void* IV = TEE_Malloc(IVlen, 0);
    TEE_CipherInit(op_dec, IV, IVlen);
    if (res != TEE_SUCCESS) {
        EMSG("Load - Cipher init failed 0x%x\n", res);
    }

    uint32_t decrypt_size = buffer_size;
    res = TEE_CipherUpdate(op_dec, enc_buffer, buffer_size, buffer, &decrypt_size);
    if (res != TEE_SUCCESS) {
        EMSG("Load - Cipher update failed 0x%x need size %d\n", res, decrypt_size);
    }

    res = TEE_CipherDoFinal(op_dec, enc_buffer, buffer_size, buffer, &decrypt_size);
    if (res != TEE_SUCCESS) {
        EMSG("Load - Cipher do final failed 0x%x need size %d\n", res, decrypt_size);
    }

    TEE_Free(IV);

    // Free operations object
    TEE_FreeOperation(op_dec);

    // convert layer buffer to layer
    TEE_AddSctrace(4201);
    layer_t* layer = buffer_to_layer(buffer, buffer_size);
    TEE_AddSctrace(4201);
    // DMSG("completed buffer to layer conversion for %d\n", layer_number);
    
    // free buffer
    TEE_Free(buffer);
    TEE_Free(enc_buffer);
    TEE_AddSctrace(420);
    return layer;
}

size_t calculate_layer_size_SHM(layer_t* layer, size_t batch_size) {
    size_t size_enc = sizeof(size_t);
    size_t int_type_size = 10 * sizeof(serialize_t);
    size_t int_data_size = 4 * sizeof(int);
    size_t matrix_type_size = 6 * sizeof(serialize_t);
    size_t matrix_meta_size = 12 * sizeof(int);
    size_t matrix_data_size = sizeof(float) * (
                                                2 * layer->weights->rows * layer->weights->cols + 
                                                2 * layer->bias->rows * layer->bias->cols + 
                                                batch_size * layer->weights->cols + 
                                                batch_size * layer->weights->rows
                                                );
    size_t total_size = size_enc + int_type_size + int_data_size + matrix_type_size + matrix_meta_size + matrix_data_size;
    return total_size;
}

// serialize layer into bytes in a buffered returned to the user
void* layer_to_buffer(layer_t* layer, size_t* out_size) {
    size_t int_type_size = 10 * sizeof(serialize_t);
    size_t int_data_size = 4 * sizeof(int);
    size_t matrix_type_size = 6 * sizeof(serialize_t);
    size_t matrix_meta_size = 12 * sizeof(int);
    size_t matrix_data_size = sizeof(float) * (
                                                layer->weights->rows * layer->weights->cols + 
                                                layer->d_weights->rows * layer->d_weights->cols +
                                                layer->bias->rows * layer->bias->cols + 
                                                layer->d_bias->rows * layer->d_bias->cols +
                                                layer->inputs->rows * layer->inputs->cols + 
                                                layer->outputs->rows * layer->outputs->cols
                                                );
    size_t total_size = int_type_size + int_data_size + matrix_type_size + matrix_meta_size + matrix_data_size;

    // DMSG("trying to allocate size %d for buffer\n", total_size);
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

// deserializes buffer into layer_t struct returned to user
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
    // if matrix is not defined, create a  1x1 matrix as placeholder
    if (!m) {
        m = create_matrix(1,1);
    }

    size_t type_size = sizeof(serialize_t);
    size_t row_size = sizeof(int), col_size = sizeof(int);
    size_t data_size = m->rows * m->cols * sizeof(float);
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
            *((float*) byte_buffer) = m->vals[i][j];
            byte_buffer += sizeof(float);
        }
    }

    *used = total_size;
}

matrix_t* deserialize_matrix(void* buffer, size_t size, size_t* used) {
    size_t type_size = sizeof(serialize_t);
    size_t row_size = sizeof(int), col_size = sizeof(int);
    size_t inter_size = type_size + row_size + col_size;

    uint8_t* byte_buffer = (uint8_t*) buffer;

    serialize_t type = *((serialize_t*) byte_buffer);
    byte_buffer += type_size;

    assert(type == matrix_s && size >= inter_size);

    int rows = *((int*) byte_buffer);
    byte_buffer += row_size;
    int cols = *((int*) byte_buffer);
    byte_buffer += col_size;

    size_t data_size = rows * cols * sizeof(float);
    size_t total_size = inter_size + data_size;

    assert(size >= total_size);

    matrix_t* m = create_matrix(rows, cols);
    for (int i=0; i < rows; ++i) {
        for (int j=0; j < cols; ++j) {
            m->vals[i][j] = *((float*) byte_buffer);
            byte_buffer += sizeof(float);
        }
    }

    *used = total_size;
    return m;

}

/*
 int w_size = sizeof(float) * layer->weights->rows * layer->weights->cols;
    int dw_size = sizeof(float) * layer->d_weights->rows * layer->d_weights->cols;
    int b_size = sizeof(float) * layer->bias->rows * layer->bias->cols; 
    int db_size = sizeof(float) * layer->d_bias->rows * layer->d_bias->cols; 
    int input_size = sizeof(float) * layer->inputs->rows * layer->inputs->cols; 
    int output_size = sizeof(float) * layer->outputs->rows * layer->outputs->cols; 
    
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
            float* tmp = buffer[curr_index];
            
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