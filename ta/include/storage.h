#ifndef TA_STORAGE_H
#define TA_STORAGE_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <network.h>

void* layer_to_buffer(layer_t* layer);

layer_t* buffer_to_layer(void* buffer);

void store_layer_to_persistence(layer_t* layer);

layer_t* read_persistence_object_to_layer(int layer_number);

void create_persistence(int layer_number, layer_t* layer);

#endif