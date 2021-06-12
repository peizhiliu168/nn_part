#ifndef DATA_H
#define DATA_H

#include <matrix.h>

typedef struct {
    int w;
    int h;
    int c;
    float *data;
} image;

typedef struct data {
    // data matrices
    matrix_t* features; // size N x feature_size
    matrix_t* labels; // size N x classes

    // data info
    int N; // number of samples
    int feature_size;
    int classes;

    // tracks current data present
    int batch_size;
    int start;
    int end;
} data_t;

image load_image(char *filename, int w, int h, int c);

void get_data_matrix_from_image_dir(char* directory, int N,
                                    int w, int h, int c, 
                                    char* identifier, int classes,
                                    matrix_t* features, matrix_t* labels);



void init_data(matrix_t* features, matrix_t* labels, int batch_size);

void destroy_data(void);

extern data_t* data_loader;

#endif