#ifndef TA_STORAGE_H
#define TA_STORAGE_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <network.h>

typedef enum {
    int_s,
    matrix_s,
} serialize_t;

void* layer_to_buffer(layer_t* layer, size_t* out_size);

layer_t* buffer_to_layer(void* buffer, size_t size);

void store_layer_to_persistence(layer_t* layer);

layer_t* read_persistence_object_to_layer(int layer_number);

void create_persistence(int layer_number, layer_t* layer);

void serialize_int(int data, void* buffer, size_t size, size_t* used);

int deserialize_int(void* buffer, size_t size, size_t* used);

void serialize_matrix(matrix_t* m, void* buffer, size_t size, size_t* used);

matrix_t* deserialize_matrix(void* buffer, size_t size, size_t* used);

#endif