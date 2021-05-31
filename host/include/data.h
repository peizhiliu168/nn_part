#ifndef DATA_H
#define DATA_H

#include <matrix.h>

typedef struct {
    int w;
    int h;
    int c;
    float *data;
} image;

image load_image(char *filename, int w, int h, int c);

void get_data_matrix_from_image_dir(char* directory, int N,
                                    int w, int h, int c, 
                                    char* identifier, int classes,
                                    matrix_t* features, matrix_t* labels);

#endif