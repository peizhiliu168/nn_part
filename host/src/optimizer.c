#include <assert.h>

#include <optimizer.h>
#include <network.h>
#include <matrix.h>
#include <math_TA.h>

void update(void) {
    assert(nn != NULL);

    switch (nn->optimizer)
    {
    case GD:
        gradient_descent_update();
        break;
    
    default:
        break;
    }
}

void gradient_descent_update(void) {
    for (int l=0; l < ((nn->n_layers - 1) / nn->n_loaded + 1); ++l) {
        int start = l * nn->n_loaded;
        int end = MIN((l + 1) * nn->n_loaded, nn->n_layers);

        swap_layers(start, end, 1);

        for (int i=0; i < (end - start); ++i) {
            layer_t* layer = nn->layers[i];

            assert(layer->d_bias != NULL && layer->bias != NULL && 
                    layer->d_weights != NULL && layer->weights != NULL);
            assert(layer->d_bias->cols == layer->bias->cols && 
                    layer->d_bias->rows == layer->bias->rows);
            assert(layer->d_weights->cols == layer->weights->cols && 
                    layer->d_weights->rows == layer->weights->rows);
            
            matrix_t* learning_rate_mat = create_matrix(1,1);
            learning_rate_mat->vals[0][0] = nn->learning_rate;

            // layer.w -= layer.d_w * self.learning_rate
            mult_matrix_element(layer->d_weights, learning_rate_mat, layer->d_weights);
            subtract_matrix_element(layer->weights, layer->d_weights, layer->weights);

            // layer.b -= layer.d_b * self.learning_rate
            mult_matrix_element(layer->d_bias, learning_rate_mat, layer->d_bias);
            subtract_matrix_element(layer->bias, layer->d_bias, layer->bias);

            destroy_matrix(learning_rate_mat);

        }
    }   
}