#include <assert.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <data.h>
#include <matrix.h>


data_t* data_loader;

void init_data(matrix_t* features, matrix_t* labels, int batch_size) {
    assert(features->rows == labels->rows);
    
    if (data_loader){
        destroy_data();
    }

    data_loader = TEE_Malloc(sizeof(data_t), TEE_MALLOC_FILL_ZERO);
    
    data_loader->features = features;
    data_loader->labels = labels;
    data_loader->N = features->rows;
    data_loader->feature_size = features->cols;
    data_loader->classes = labels->cols;
    data_loader->batch_size = batch_size;
    data_loader->start = 0;
    data_loader->end = data_loader->start + batch_size;
}

// destory the data loader
void destroy_data(void) {
    if (!data_loader){
        return;
    }

    destroy_matrix(data_loader->features);
    destroy_matrix(data_loader->labels);

    TEE_Free(data_loader);
}

// wraps data from vector into a matrix
matrix_t* wrap_data(float* data, int size, int rows, int cols) {
    // DMSG("size: %d, rows: %d, cols: %d\n", size, rows, cols);
    assert(size == (rows * cols));

    matrix_t* wrapped = create_matrix(rows, cols);
    for (int i=0; i < rows; ++i) {
        for (int j=0; j < cols; ++j) {
            wrapped->vals[i][j] = data[cols * i + j];
        }
    }

    return wrapped;
}