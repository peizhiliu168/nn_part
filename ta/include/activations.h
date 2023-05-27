#ifndef TA_ACTIVATIONS_H
#define TA_ACTIVATIONS_H

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

float sigmoid(float x);

float d_sigmoid(float x);

float relu(float x);

float d_relu(float x);

#endif