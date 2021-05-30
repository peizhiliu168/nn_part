#ifndef DATA_H
#define DATA_H

typedef struct {
    int w;
    int h;
    int c;
    float *data;
} image;

typedef struct matrix {
    int rows;
    int cols;
    double** vals;
} matrix_t;

image load_image(char *filename, int w, int h, int c);

#endif