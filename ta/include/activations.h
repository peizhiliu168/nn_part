#ifndef TA_ACTIVATIONS_H
#define TA_ACTIVATIONS_H

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

double sigmoid(double x);

double d_sigmoid(double x);

double relu(double x);

double d_relu(double x);

#endif